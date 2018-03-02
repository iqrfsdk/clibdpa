/**
 * Copyright 2015-2017 MICRORISC s.r.o.
 * Copyright 2017 IQRF Tech s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TaskQueue.h"
#include "DpaHandler2.h"
#include "DpaMessage.h"
#include "IqrfLogging.h"

#include <future>

/////////////////////////////////////
class DpaTransactionResult2 : public IDpaTransactionResult2
{
public:
  DpaTransactionResult2()
  {
  }

  DpaTransactionResult2(const DpaMessage& request)
  {
    //TODO if not request?
    m_request_ts = std::chrono::system_clock::now();
    m_request = request;
  }

  int getTransactionResult() const override
  {
    return m_transactionResult;
  }

  int getDpaResult() const override
  {
    return m_dpaResult;
  }

  const std::string& getResultString() const override
  {
    static std::string ret("not implemented yet");
    return ret;
  }

  const DpaMessage& getRequest() const override
  {
    return m_request;
  }

  const DpaMessage& getConfirmation() const override
  {
    return m_confirmation;
  }

  const DpaMessage& getResponse() const override
  {
    return m_response;
  }

  const std::chrono::time_point<std::chrono::system_clock>& getRequestTs() const override 
  {
    return m_request_ts;
  }

  const std::chrono::time_point<std::chrono::system_clock>& getConfirmationTs() const override
  {
    return m_confirmation_ts;
  }

  const std::chrono::time_point<std::chrono::system_clock>& getResponseTs() const override
  {
    return m_response_ts;
  }

  void setConfirmation(const DpaMessage& confirmation)
  {
    //TODO if not confirmation?
    m_confirmation_ts = std::chrono::system_clock::now();
    m_confirmation = confirmation;
  }

  void setResponse(const DpaMessage& response)
  {
    //TODO if not response?
    m_response_ts = std::chrono::system_clock::now();
    m_response = response;
  }

private:
  DpaMessage m_request;
  DpaMessage m_response;
  DpaMessage m_confirmation;
  std::chrono::time_point<std::chrono::system_clock> m_request_ts;
  std::chrono::time_point<std::chrono::system_clock> m_confirmation_ts;
  std::chrono::time_point<std::chrono::system_clock> m_response_ts;
  int m_transactionResult = 0;
  int m_dpaResult = 0;
};

/////////////////////////////////////
class DpaTransaction2 : public IDpaTransaction2
{
public:
  /// Asynchronous DPA message handler functional type
  typedef std::function<void(const DpaMessage& dpaMessage)> SendDpaMessageFunc;

  DpaTransaction2(const DpaMessage& request, SendDpaMessageFunc sender)
    :m_sender(sender)
    ,m_request(request)
  {
  }

  std::unique_ptr<IDpaTransactionResult2> get(uint32_t timeout) override
  {
    m_future = m_promise.get_future();
    m_timeout = timeout;
    int m_error = 0;

    // infinite timeout
    if (timeout <= 0) {
      // blocks until the result becomes available
      m_future.wait();
      m_error = m_future.get();
    }
    else {
      std::chrono::milliseconds span(timeout * 2);
      if (m_future.wait_for(span) == std::future_status::timeout) {
        // future error
        TRC_ERR("Transaction task future timeout.");
        m_error = -2;
      }
      else {
        m_error = m_future.get();
      }
    }

    //TODO move?
    std::unique_ptr<IDpaTransactionResult2> result(new DpaTransactionResult2(m_dpaTransactionResult));
    return result;
  }

  //virtual void processConfirmationMessage(const DpaMessage& confirmation) = 0;
  //virtual void processResponseMessage(const DpaMessage& response) = 0;
  //virtual void processFinish(DpaTransfer::DpaTransferStatus status) = 0;

  void execute()
  {
    const DpaMessage& message = m_request;

    int32_t remains(0);
    //TODO timeout magic
    int32_t defaultTimeout = m_timeout;
    int32_t requiredTimeout = m_timeout;

    if (requiredTimeout < 0) {
      requiredTimeout = defaultTimeout;
      TRC_DBG("Using default timeout: " << PAR(defaultTimeout));
    }

    if (requiredTimeout < DpaHandler2::MINIMAL_TIMING) {
      //it is allowed just for Coordinator Discovery
      if (message.DpaPacket().DpaRequestPacket_t.NADR != COORDINATOR_ADDRESS ||
        message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_COORDINATOR_DISCOVERY) {
        // force setting minimal timing as only Discovery can have infinite timeout
        TRC_WAR("Explicit: " << PAR(requiredTimeout) << "forced to: " << PAR(DpaHandler2::MINIMAL_TIMING));
        requiredTimeout = DpaHandler2::MINIMAL_TIMING;
      }
      else {
        TRC_WAR(PAR(requiredTimeout) << "allowed for DISCOVERY message");
      }
    }

    // update transfer state
    DpaTransfer::DpaTransferStatus status(DpaTransfer::kCreated);

    try {
      m_currTransfer.DefaultTimeout(m_timeout);
      m_sender(message);
      m_currTransfer.ProcessSentMessage(message);

      // waits till message processed - kProcessed
      //int expectedDuration = 0;
      //return m_currentTransfer.IsInProgress(expectedDuration);

      while (m_currTransfer.IsInProgress(remains)) {
        //waits for remaining time or notifing
        {
          if (remains > 0) {
            TRC_DBG("Conditional wait - time to wait yet: " << PAR(remains));
          }
          else {
            // polutes tracer file if DISCOVERY is run 
            //TRC_DBG("Conditional wait - time is out: " << PAR(remains));
          }

          std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
          m_conditionVariable.wait_for(lck, std::chrono::milliseconds(remains));
        }
      }

      // update transfer state
      status = m_currTransfer.ProcessStatus();
    }
    catch (std::exception& e) {
      TRC_WAR("Send error occured: " << e.what());
      status = DpaTransfer::DpaTransferStatus::kError;
    }

    //Timeout(defaultTimeout);
    m_timeout = defaultTimeout;
    processFinish(status);
  }

  void ProcessReceivedMessage(const DpaMessage& request)
  {
    m_currTransfer.ProcessReceivedMessage(request);
  }

private:
  void processFinish(DpaTransfer::DpaTransferStatus status)
  {
    // set error value
    //switch (status) {
    //case DpaTransfer::DpaTransferStatus::kError:
    //  m_error = -4;
    //  break;
    //case DpaTransfer::DpaTransferStatus::kAborted:
    //  m_error = -3;
    //  break;
    //case DpaTransfer::DpaTransferStatus::kTimeout:
    //  m_error = -1;
    //  break;
    //default:;
    //}

    // sync with future
    //m_promise.set_value(m_error);
    m_promise.set_value(0);
  }

  DpaTransactionResult2 m_dpaTransactionResult;
  DpaMessage m_request;

  //The promise object is the asynchronous provider and is expected to set a value for the shared state at some point.
  //The future object is an asynchronous return object that can retrieve the value of the shared state, waiting for it to be ready, if necessary.
  std::promise<int> m_promise;
  std::future<int> m_future;
  SendDpaMessageFunc m_sender;
  int m_timeout = 0;
  DpaTransfer m_currTransfer;
  std::mutex m_conditionVariableMutex;
  std::condition_variable m_conditionVariable;
};


/////////////////////////////////////
class DpaHandler2::Imp
{
public:
  Imp(IChannel* iqrfInterface)
    :m_iqrfInterface(iqrfInterface)
  {
    m_dpaTransactionQueue = new TaskQueue<std::shared_ptr<DpaTransaction2>>([&](std::shared_ptr<DpaTransaction2> ptr) {
      m_pendingTransaction = ptr;
      m_pendingTransaction->execute();
    });

    if (iqrfInterface == nullptr) {
      throw std::invalid_argument("DPA interface argument can not be nullptr.");
    }
    m_iqrfInterface = iqrfInterface;

    // default timeout 200ms
    //m_defaultTimeoutMs = kDefaultTimeout;
    //TRC_INF("Ctor default user timeout: " << PAR(m_defaultTimeoutMs));

    // register callback for cdc or spi interface
    m_iqrfInterface->registerReceiveFromHandler([&](const std::basic_string<unsigned char>& msg) -> int {
      ResponseMessageHandler(msg);
      return 0;
    });

  }

  ~Imp()
  {
    m_dpaTransactionQueue->stopQueue();
    delete m_dpaTransactionQueue;
  }

  // any received message from the channel
  void ResponseMessageHandler(const std::basic_string<unsigned char>& message) {
    if (message.length() == 0)
      return;

    // signal that message is received, but not yet processed
    //m_currentTransfer->MessageReceived(true);
    // there were delays sometime before processing causing timeout and not processing response at all
    //m_messageToBeProcessed = flg;

    TRC_DBG(">>>>>>>>>>>>>>>>>>" << std::endl <<
      "Received from IQRF interface: " << std::endl << FORM_HEX(message.data(), message.length()));

    // new message
    DpaMessage receivedMessage;
    try {
      receivedMessage.FillFromResponse(message.data(), message.length());
    }
    catch (std::exception& e) {
      //m_currentTransfer->MessageReceived(false);
      // there were delays sometime before processing causing timeout and not processing response at all
      //m_messageToBeProcessed = flg;
      CATCH_EX("in processing msg", std::exception, e);
      return;
    }

    auto messageDirection = receivedMessage.MessageDirection();
    if (messageDirection == DpaMessage::MessageType::kRequest) {
      //Always Async
      //m_currentTransfer->MessageReceived(false);
      // there were delays sometime before processing causing timeout and not processing response at all
      //m_messageToBeProcessed = flg;
      processAsynchronousMessage(message);
      return;
    }
    else if (messageDirection == DpaMessage::MessageType::kResponse &&
      receivedMessage.DpaPacket().DpaResponsePacket_t.ResponseCode & STATUS_ASYNC_RESPONSE) {
      // async msg
      //m_currentTransfer->MessageReceived(false);
      processAsynchronousMessage(message);
      return;
    }
    else {
      try {
        // transfer msg
        m_pendingTransaction->ProcessReceivedMessage(message);
        // notification about reception
        //{
        //  std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
        //  m_conditionVariable.notify_one();
        //  TRC_INF("Notify from ResponseMessageHandler: message received.");
        //}
      }
      catch (std::logic_error& le) {
        CATCH_EX("Process received message error...", std::logic_error, le);
      }
    }
  }

  std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request)
  {
    std::shared_ptr<DpaTransaction2> ptr(new DpaTransaction2(request,
      [&](const DpaMessage& r) {
        sendRequest(r);
      }
    ));
    m_dpaTransactionQueue->pushToQueue(ptr);
    return ptr;
  }

  void killDpaTransaction()
  {

  }

  int getTimeout() const
  {
    return m_timeout;
  }

  void setTimeout(int timeout)
  {
    m_timeout = timeout;
  }

  RfMode getRfCommunicationMode() const
  {
    return m_rfMode;
  }

  void setRfCommunicationMode(RfMode rfMode)
  {
    m_rfMode = rfMode;
  }

  void registerAsyncMessageHandler(const std::string& serviceId, AsyncMessageHandlerFunc fun)
  {
    //TODO serviceId?
    std::lock_guard<std::mutex> lck(m_asyncMessageMutex);
    m_asyncMessageHandler = fun;
  }

  void processAsynchronousMessage(const DpaMessage& message) {
    m_asyncMessageMutex.lock();

    if (m_asyncMessageHandler) {
      m_asyncMessageHandler(message);
    }

    m_asyncMessageMutex.unlock();
  }

  void unregisterAsyncMessageHandler(const std::string& serviceId)
  {
    //TODO serviceId?
    std::lock_guard<std::mutex> lck(m_asyncMessageMutex);
    m_asyncMessageHandler = nullptr;
  }

private:
  void sendRequest(const DpaMessage& request)
  {
    TRC_DBG("<<<<<<<<<<<<<<<<<<" << std::endl <<
      "Sent to DPA interface: " << std::endl << FORM_HEX(request.DpaPacketData(), request.GetLength()));
    m_iqrfInterface->sendTo(std::basic_string<unsigned char>(request.DpaPacketData(), request.GetLength()));
  }

  RfMode m_rfMode = RfMode::Std;
  AsyncMessageHandlerFunc m_asyncMessageHandler;
  std::mutex m_asyncMessageMutex;
  IChannel* m_iqrfInterface = nullptr;
  int m_timeout;

  //TODO queue
  std::shared_ptr<DpaTransaction2> m_pendingTransaction;
  TaskQueue<std::shared_ptr<DpaTransaction2>>* m_dpaTransactionQueue = nullptr;
};

/////////////////////////////
DpaHandler2::DpaHandler2(IChannel* iqrfInterface)
{
  m_imp = new Imp(iqrfInterface);
}

DpaHandler2::~DpaHandler2()
{
  delete m_imp;
}

std::shared_ptr<IDpaTransaction2> DpaHandler2::executeDpaTransaction(const DpaMessage& request)
{
  return m_imp->executeDpaTransaction(request);
}

void DpaHandler2::killDpaTransaction()
{
  m_imp->killDpaTransaction();
}

int DpaHandler2::getTimeout() const
{
  return m_imp->getTimeout();
}

void DpaHandler2::setTimeout(int timeout)
{
  m_imp->setTimeout(timeout);
}

IDpaHandler2::RfMode DpaHandler2::getRfCommunicationMode() const
{
  return m_imp->getRfCommunicationMode();
}

void DpaHandler2::setRfCommunicationMode(IDpaHandler2::RfMode rfMode)
{
  m_imp->setRfCommunicationMode(rfMode);
}

void DpaHandler2::registerAsyncMessageHandler(const std::string& serviceId, IDpaHandler2::AsyncMessageHandlerFunc fun)
{
  m_imp->registerAsyncMessageHandler(serviceId, fun);
}

void DpaHandler2::unregisterAsyncMessageHandler(const std::string& serviceId)
{
  m_imp->unregisterAsyncMessageHandler(serviceId);
}
