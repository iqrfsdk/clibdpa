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

#include "unexpected_command.h"
#include "unexpected_packet_type.h"
#include "unexpected_peripheral.h"

#include <future>
 

/** Values that represent DPA transfer state. */
enum DpaTransfer2Status
{
  kCreated,
  ///< An enum constant representing the first message was sent.
  kSent,
  kSentCoordinator,
  ///< An enum constant representing the confirmation, broadcast, response was received.
  kConfirmation,
  kConfirmationBroadcast,
  kReceivedResponse,
  ///< An enum constant representing the whole transfer was processed.
  kProcessed,
  ///< An enum constant representing the timeout expired.
  kTimeout,
  ///< An enum constant representing the transfer was aborted.
  kAborted,
  ///< An enum constant representing the error state during sending via iqrf interface.
  kError
};

/////////////////////////////////////
// class DpaTransactionResult2
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

  int getErrorCode() const override
  {
    return m_errorCode;
  }

  std::string getErrorString() const override
  {
    switch (m_errorCode) {
    case -4:
      return "ERROR_IFACE";
    case -3:
      return "ERROR_ABORTED";
    case -2:
      return "ERROR_PROMISE_TIMEOUT";
    case -1:
      return "ERROR_TIMEOUT";
    case STATUS_NO_ERROR:
      return "STATUS_NO_ERROR";
    case ERROR_FAIL:
      return "ERROR_FAIL";
    case ERROR_PCMD:
      return "ERROR_PCMD";
    case ERROR_PNUM:
      return "ERROR_PNUM";
    case ERROR_ADDR:
      return "ERROR_ADDR";
    case ERROR_DATA_LEN:
      return "ERROR_DATA_LEN";
    case ERROR_DATA:
      return "ERROR_DATA";
    case ERROR_HWPID:
      return "ERROR_HWPID";
    case ERROR_NADR:
      return "ERROR_NADR";
    case ERROR_IFACE_CUSTOM_HANDLER:
      return "ERROR_IFACE_CUSTOM_HANDLER";
    case ERROR_MISSING_CUSTOM_DPA_HANDLER:
      return "ERROR_MISSING_CUSTOM_DPA_HANDLER";
    case ERROR_USER_TO:
      return "ERROR_USER_TO";
    case STATUS_CONFIRMATION:
      return "STATUS_CONFIRMATION";
    case ERROR_USER_FROM:
    default:
      std::ostringstream os;
      os << std::hex << m_errorCode;
      return os.str();
    }
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

  void setErrorCode(int errorCode)
  {
    m_errorCode = errorCode;
  }

private:
  DpaMessage m_request;
  DpaMessage m_response;
  DpaMessage m_confirmation;
  std::chrono::time_point<std::chrono::system_clock> m_request_ts;
  std::chrono::time_point<std::chrono::system_clock> m_confirmation_ts;
  std::chrono::time_point<std::chrono::system_clock> m_response_ts;
  int m_errorCode = 0;
};

/////////////////////////////////////
// class DpaTransaction2
/////////////////////////////////////
class DpaTransaction2 : public IDpaTransaction2
{
public:
  /// Asynchronous DPA message handler functional type
  typedef std::function<void(const DpaMessage& dpaMessage)> SendDpaMessageFunc;

  DpaTransaction2(const DpaMessage& request, SendDpaMessageFunc sender)
    :m_sender(sender)
    ,m_dpaTransactionResultPtr(new DpaTransactionResult2(request))
    //DpaTransfer
    ,m_status(kCreated)
    ,m_messageToBeProcessed(false)
    ,m_expectedDurationMs(200), m_timeoutMs(200), m_currentCommunicationMode(IDpaHandler2::RfMode::kStd)
  {
  }

  virtual ~DpaTransaction2()
  {
  }

  std::unique_ptr<IDpaTransactionResult2> get(uint32_t timeout) override
  {
    m_future = m_promise.get_future();
    m_timeout = timeout;
    int errorCode = 0;

    // infinite timeout
    if (timeout <= 0) {
      // blocks until the result becomes available
      m_future.wait();
      errorCode = m_future.get();
    }
    else {
      std::chrono::milliseconds span(timeout * 2);
      if (m_future.wait_for(span) == std::future_status::timeout) {
        // future error
        TRC_ERR("Transaction task future timeout.");
        errorCode = -2;
      }
      else {
        errorCode = m_future.get();
      }
    }

    m_dpaTransactionResultPtr->setErrorCode(errorCode);
    //TODO move?
    //std::unique_ptr<IDpaTransactionResult2> result(new DpaTransactionResult2(m_dpaTransactionResult));
    std::unique_ptr<IDpaTransactionResult2> result = std::move(m_dpaTransactionResultPtr);
    return result;
  }

  void execute()
  {
    const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();

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
    DpaTransfer2Status status(kCreated);

    try {
      //m_currTransfer.DefaultTimeout(m_timeout);
      DefaultTimeout(m_timeout);
      m_sender(message);
      //m_currTransfer.ProcessSentMessage(message);
      ProcessSentMessage(message);

      //while (m_currTransfer.IsInProgress(remains)) {
      while (IsInProgress(remains)) {
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
      //status = m_currTransfer.ProcessStatus();
      status = ProcessStatus();
    }
    catch (std::exception& e) {
      TRC_WAR("Send error occured: " << e.what());
      status = DpaTransfer2Status::kError;
    }

    // reset timeout
    m_timeout = defaultTimeout;
    
    // set error value
    switch (status) {
    case DpaTransfer2Status::kError:
      m_dpaTransactionResultPtr->setErrorCode(-4);
      break;
    case DpaTransfer2Status::kAborted:
      m_dpaTransactionResultPtr->setErrorCode(-3);
      break;
    case DpaTransfer2Status::kTimeout:
      m_dpaTransactionResultPtr->setErrorCode(-1);
      break;
    default:;
    }

    // sync with future
    m_promise.set_value(m_dpaTransactionResultPtr->getErrorCode());
  }

private:
  std::unique_ptr<DpaTransactionResult2> m_dpaTransactionResultPtr;

  //The promise object is the asynchronous provider and is expected to set a value for the shared state at some point.
  //The future object is an asynchronous return object that can retrieve the value of the shared state, waiting for it to be ready, if necessary.
  std::promise<int> m_promise;
  std::future<int> m_future;
  SendDpaMessageFunc m_sender;
  int m_timeout = 0;
  std::mutex m_conditionVariableMutex;
  std::condition_variable m_conditionVariable;

//*******************************************************************
//DpaTransfer2
//*******************************************************************
  int32_t DefaultTimeout() const
  {
    return m_timeoutMs;
  }

  void DefaultTimeout(int32_t timeoutMs)
  {
    this->m_timeoutMs = timeoutMs;
  }

  void ProcessSentMessage(const DpaMessage& sentMessage)
  {
    TRC_ENTER("");

    if (m_status != kCreated)
    {
      throw std::logic_error("Sent message already set.");
    }

    // current request status is set as sent
    if (sentMessage.NodeAddress() == COORDINATOR_ADDRESS) {
      SetStatus(kSentCoordinator);
    }
    else {
      SetStatus(kSent);
    }

    // setting default timeout, no estimation yet
    SetTimingForCurrentTransfer();

    TRC_LEAVE("");
  }

  void MessageReceived(bool flg) {
    // there were delays sometime before processing causing timeout and not processing response at all
    m_messageToBeProcessed = flg;
  }

public:
  void ProcessReceivedMessage(const DpaMessage& receivedMessage)
  {
    TRC_ENTER("");
    // direction
    auto messageDirection = receivedMessage.MessageDirection();

    // is transfer in progress?
    if (!IsInProgressStatus(m_status)) {
      m_messageToBeProcessed = false;
      return;
    }
    // yes
    else {
      // no request is expected
      if (messageDirection != DpaMessage::kResponse && messageDirection != DpaMessage::kConfirmation) {
        // clear flag after processing
        m_messageToBeProcessed = false;
        throw unexpected_packet_type("Response is expected.");
      }
      const DpaMessage& request = m_dpaTransactionResultPtr->getRequest();
      // same as sent request
      if (receivedMessage.NodeAddress() != request.NodeAddress()) {
        // clear flag after processing
        m_messageToBeProcessed = false;
        throw unexpected_peripheral("Different node address than in sent message.");
      }
      // same as sent request
      if (receivedMessage.PeripheralType() != request.PeripheralType()) {
        // clear flag after processing
        m_messageToBeProcessed = false;
        throw unexpected_peripheral("Different peripheral type than in sent message.");
      }
      // same as sent request
      if ((receivedMessage.PeripheralCommand() & ~0x80) != request.PeripheralCommand()) {
        // clear flag after processing
        m_messageToBeProcessed = false;
        throw unexpected_command("Different peripheral command than in sent message.");
      }
    }

    if (messageDirection == DpaMessage::kConfirmation) {
      ProcessConfirmationMessage(receivedMessage);
      TRC_INF("Confirmation processed.");
    }
    else {
      ProcessResponseMessage(receivedMessage);
      TRC_INF("Response processed.");
    }

    // clear flag after processing
    m_messageToBeProcessed = false;
    TRC_LEAVE("");
  }

private:
  void ProcessConfirmationMessage(const DpaMessage& confirmationMessage)
  {
    if (confirmationMessage.NodeAddress() == DpaMessage::kBroadCastAddress) {
      SetStatus(kConfirmationBroadcast);
    }
    else {
      SetStatus(kConfirmation);
    }

    m_dpaTransactionResultPtr->setConfirmation(confirmationMessage);

    // setting timeout based on the confirmation
    SetTimingForCurrentTransfer(EstimatedTimeout(confirmationMessage));
  }

  void ProcessResponseMessage(const DpaMessage& responseMessage)
  {
    // if there is a request to coordinator then after receiving response it is allowed to send another
    if (m_status == kSentCoordinator) {
      // done, next request gets ready 
      SetStatus(kProcessed);
    }
    else {
      // only if there is not infinite timeout
      if (m_expectedDurationMs != 0) {
        SetStatus(kReceivedResponse);
        // adjust timing before allowing next request
        SetTimingForCurrentTransfer(EstimatedTimeout(responseMessage));
      }
      // infinite timeout
      else {
        // done, next request gets ready 
        SetStatus(kProcessed);
      }
    }

    m_dpaTransactionResultPtr->setResponse(responseMessage);
  }

  int32_t EstimatedTimeout(const DpaMessage& receivedMessage)
  {
    // direction
    auto direction = receivedMessage.MessageDirection();

    // double check
    if (direction != DpaMessage::kConfirmation && direction != DpaMessage::kResponse) {
      throw std::invalid_argument("Parameter is not a received message type.");
    }

    // confirmation
    if (direction == DpaMessage::kConfirmation) {
      auto iFace = receivedMessage.DpaPacket().DpaResponsePacket_t.DpaMessage.IFaceConfirmation;

      // save for later use with response
      m_hops = iFace.Hops;
      m_timeslotLength = iFace.TimeSlotLength;
      m_hopsResponse = iFace.HopsResponse;

      // lp
      if (m_currentCommunicationMode == IDpaHandler2::RfMode::kLp) {
        return EstimateLpTimeout(m_hops, m_timeslotLength, m_hopsResponse);
      }
      // std
      return EstimateStdTimeout(m_hops, m_timeslotLength, m_hopsResponse);
    }

    // response
    if (direction == DpaMessage::kResponse) {
      // lp
      if (m_currentCommunicationMode == IDpaHandler2::RfMode::kLp) {
        return EstimateLpTimeout(m_hops, m_timeslotLength, m_hopsResponse,
          receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2));
      }
      // std
      return EstimateStdTimeout(m_hops, m_timeslotLength, m_hopsResponse,
        receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2));
    }
  }

  int32_t EstimateStdTimeout(uint8_t hopsRequest, uint8_t timeslotReq, uint8_t hopsResponse, int8_t responseDataLength = -1)
  {
    TRC_ENTER("");
    int32_t responseTimeSlotLengthMs;

    auto estimatedTimeoutMs = (hopsRequest + 1) * timeslotReq * 10;

    // estimation from confirmation 
    if (responseDataLength == -1) {
      if (timeslotReq == 20) {
        responseTimeSlotLengthMs = 200;
      }
      else {
        // worst case
        responseTimeSlotLengthMs = 60;
      }
    }
    // correction of the estimation from response 
    else {
      TRC_DBG("PData length of the received response: " << PAR((int)responseDataLength));
      if (responseDataLength >= 0 && responseDataLength < 16)
      {
        responseTimeSlotLengthMs = 40;
      }
      else if (responseDataLength >= 16 && responseDataLength <= 39)
      {
        responseTimeSlotLengthMs = 50;
      }
      else if (responseDataLength > 39)
      {
        responseTimeSlotLengthMs = 60;
      }
      TRC_DBG("Correction of the response timeout: " << PAR(responseTimeSlotLengthMs));
    }

    estimatedTimeoutMs += (hopsResponse + 1) * responseTimeSlotLengthMs + m_safetyTimeoutMs;

    TRC_INF("Estimated STD timeout: " << PAR(estimatedTimeoutMs));
    TRC_LEAVE("");
    return estimatedTimeoutMs;
  }

  int32_t EstimateLpTimeout(uint8_t hopsRequest, uint8_t timeslotReq, uint8_t hopsResponse, int8_t responseDataLength = -1)
  {
    TRC_ENTER("");
    int32_t responseTimeSlotLengthMs;

    auto estimatedTimeoutMs = (hopsRequest + 1) * timeslotReq * 10;

    // estimation from confirmation 
    if (responseDataLength == -1) {
      if (timeslotReq == 20) {
        responseTimeSlotLengthMs = 200;
      }
      else {
        // worst case
        responseTimeSlotLengthMs = 110;
      }
    }
    // correction of the estimation from response 
    else {
      TRC_DBG("PData length of the received response: " << PAR((int)responseDataLength));
      if (responseDataLength >= 0 && responseDataLength < 11)
      {
        responseTimeSlotLengthMs = 80;
      }
      else if (responseDataLength >= 11 && responseDataLength <= 33)
      {
        responseTimeSlotLengthMs = 90;
      }
      else if (responseDataLength >= 34 && responseDataLength <= 56)
      {
        responseTimeSlotLengthMs = 100;
      }
      else if (responseDataLength > 56)
      {
        responseTimeSlotLengthMs = 110;
      }
      TRC_DBG("Correction of the response timeout: " << PAR(responseTimeSlotLengthMs));
    }

    estimatedTimeoutMs += (hopsResponse + 1) * responseTimeSlotLengthMs + m_safetyTimeoutMs;

    TRC_INF("Estimated LP timeout: " << PAR(estimatedTimeoutMs));
    TRC_LEAVE("");
    return estimatedTimeoutMs;
  }

  void SetTimingForCurrentTransfer(int32_t estimatedTimeMs = 0)
  {
    TRC_ENTER("");

    // waiting forever
    if (m_timeoutMs == 0) {
      m_expectedDurationMs = m_timeoutMs;
      TRC_INF("Expected duration to wait :" << PAR(m_expectedDurationMs));
      return;
    }

    // adjust time to wait before allowing next request to go the iqrf network
    if (m_status == kReceivedResponse) {
      //adjust new timing based on length of PData in response
      m_expectedDurationMs = estimatedTimeMs;
      TRC_INF("New expected duration to wait :" << PAR(m_expectedDurationMs));
      return;
    }

    // estimation done
    if (estimatedTimeMs >= 0) {
      // either default timeout is 200 or user sets lower time than estimated
      if (m_timeoutMs < estimatedTimeMs) {
        // in both cases use estimation from confirmation
        m_timeoutMs = estimatedTimeMs;
      }
      // set new duration
      // there is also case when user sets higher than estimation then user choice is set
      m_expectedDurationMs = m_timeoutMs;
      TRC_INF("Expected duration to wait :" << PAR(m_expectedDurationMs));
    }

    // start time when dpa request is sent and rerun again when confirmation is received
    m_startTime = std::chrono::system_clock::now();
    TRC_INF("Transfer status: started");

    TRC_LEAVE("");
  }

  DpaTransfer2Status ProcessStatus() {
    TRC_ENTER("");

    // changes m_status, does not care about remains
    // todo: refactor and rename - two functions
    CheckTimeout();

    TRC_LEAVE(PAR(m_status));
    return m_status;
  }

  int32_t CheckTimeout()
  {
    int32_t remains(0);

    if (m_status == kCreated) {
      TRC_INF("Transfer status: created");
      return remains;
    }

    if (m_status == kAborted) {
      TRC_INF("Transfer status: aborted");
      return remains;
    }

    bool timingFinished(false);

    // infinite (0) is out of this statement
    if (m_expectedDurationMs > 0) {
      // passed time from sent request
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_startTime);
      remains = m_expectedDurationMs - duration.count();
      TRC_INF("Time to wait: " << PAR(remains));

      // already over?
      timingFinished = remains < 0;
    }

    // not yet set and yes time is over
    // processed or timeouted can be set only after finished timing
    if (m_status != kProcessed && m_status != kTimeout) {
      if (timingFinished) {
        // double check that there is no message yet to be processed
        // this allows not to exit transfer before actual processing
        if (!m_messageToBeProcessed) {
          // and we have received confirmation for broadcast or response
          if (m_status == kConfirmationBroadcast || m_status == kReceivedResponse) {
            SetStatus(kProcessed);
            TRC_INF("Transfer status: processed");
          }
          else {
            SetStatus(kTimeout);
            TRC_INF("Transfer status: timeout");
          }
        }
      }
    }

    // time to wait
    return remains;
  }

  bool IsInProgress() {
    return IsInProgressStatus(ProcessStatus());
  }

  bool IsInProgress(int32_t& expectedDuration) {
    expectedDuration = CheckTimeout();
    return IsInProgressStatus(m_status);
  }

  bool IsInProgressStatus(DpaTransfer2Status status) {
    switch (status)
    {
    case kSent:
    case kSentCoordinator:
    case kConfirmation:
    case kConfirmationBroadcast:
    case kReceivedResponse:
      return true;
      // kCreated, kProcessed, kTimeout, kAbort, kError
    default:
      return false;
    }
  }

  void Abort() {
    std::lock_guard<std::mutex> lck(m_statusMutex);
    m_status = kAborted;
  }

  void SetStatus(DpaTransfer2Status status)
  {
    TRC_ENTER("");
    {
      std::lock_guard<std::mutex> lck(m_statusMutex);
      m_status = status;
    }
    TRC_LEAVE("");
  }


private: //DpaTransfer2
  /** An extra timeout added to timeout from a confirmation packet. */
  const int32_t m_safetyTimeoutMs = 40;

  std::mutex m_statusMutex;
  IDpaHandler2::RfMode m_currentCommunicationMode;

  std::chrono::system_clock::time_point m_startTime;
  int32_t m_timeoutMs;
  int32_t m_expectedDurationMs;

  DpaTransfer2Status m_status;
  //DpaMessage* m_sentMessage = nullptr;

  int8_t m_hops;
  int8_t m_timeslotLength;
  int8_t m_hopsResponse;

  bool m_messageToBeProcessed;

};

/////////////////////////////////////
// class DpaHandler2::Imp
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

  RfMode m_rfMode = RfMode::kStd;
  AsyncMessageHandlerFunc m_asyncMessageHandler;
  std::mutex m_asyncMessageMutex;
  IChannel* m_iqrfInterface = nullptr;
  int m_timeout;

  //TODO queue
  std::shared_ptr<DpaTransaction2> m_pendingTransaction;
  TaskQueue<std::shared_ptr<DpaTransaction2>>* m_dpaTransactionQueue = nullptr;

};

/////////////////////////////////////
// class DpaHandler2
/////////////////////////////////////
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
