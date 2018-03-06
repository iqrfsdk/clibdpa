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
 
/////////////////////////////////////
// class DpaTransactionResult2
/////////////////////////////////////
class DpaTransactionResult2 : public IDpaTransactionResult2
{
private:
  /// original request creating the transaction
  DpaMessage m_request;
  /// received  confirmation
  DpaMessage m_confirmation;
  /// received  response
  DpaMessage m_response;
  /// request timestamp
  std::chrono::time_point<std::chrono::system_clock> m_request_ts;
  /// confirmation timestamp
  std::chrono::time_point<std::chrono::system_clock> m_confirmation_ts;
  /// response timestamp
  std::chrono::time_point<std::chrono::system_clock> m_response_ts;
  /// overall error code
  int m_errorCode = 0;

public:
  DpaTransactionResult2() = delete;
  DpaTransactionResult2(const DpaMessage& request)
  {
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
    case TRN_ERROR_IFACE_QUEUE_FULL:
      return "ERROR_IFACE_QUEUE_FULL";
    case TRN_ERROR_IFACE:
      return "ERROR_IFACE";
    case TRN_ERROR_ABORTED:
      return "ERROR_ABORTED";
    case TRN_ERROR_IFACE_BUSY:
      return "ERROR_IFACE_BUSY";
    case TRN_ERROR_TIMEOUT:
      return "ERROR_TIMEOUT";
    case TRN_OK:
      return "OK";
    case TRN_ERROR_FAIL:
      return "ERROR_FAIL";
    case TRN_ERROR_PCMD:
      return "ERROR_PCMD";
    case TRN_ERROR_PNUM:
      return "ERROR_PNUM";
    case TRN_ERROR_ADDR:
      return "ERROR_ADDR";
    case TRN_ERROR_DATA_LEN:
      return "ERROR_DATA_LEN";
    case TRN_ERROR_DATA:
      return "ERROR_DATA";
    case TRN_ERROR_HWPID:
      return "ERROR_HWPID";
    case TRN_ERROR_NADR:
      return "ERROR_NADR";
    case TRN_ERROR_IFACE_CUSTOM_HANDLER:
      return "ERROR_IFACE_CUSTOM_HANDLER";
    case TRN_ERROR_MISSING_CUSTOM_DPA_HANDLER:
      return "ERROR_MISSING_CUSTOM_DPA_HANDLER";
    case TRN_ERROR_USER_TO:
      return "ERROR_USER_TO";
    case TRN_STATUS_CONFIRMATION:
      return "STATUS_CONFIRMATION";
    case TRN_ERROR_USER_FROM:
    default:
      std::ostringstream os;
      os << "TRN_ERROR_USER_" << std::hex << m_errorCode;
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
    m_confirmation_ts = std::chrono::system_clock::now();
    m_confirmation = confirmation;
  }

  void setResponse(const DpaMessage& response)
  {
    m_response_ts = std::chrono::system_clock::now();
    m_response = response;
  }

  void setErrorCode(int errorCode)
  {
    m_errorCode = errorCode;
  }
};

/////////////////////////////////////
// class DpaTransaction2
/////////////////////////////////////
class DpaTransaction2 : public IDpaTransaction2
{
public:
  /// type of functor to send the request message towards the coordinator
  typedef std::function<void(const DpaMessage& dpaMessage)> SendDpaMessageFunc;

private:
  //// Values that represent transaction state.
  enum DpaTransfer2State
  {
    /// init state
    kCreated,
    /// request was sent.
    kSent,
    /// request with coordinator address was sent.
    kSentCoordinator,
    /// confirmation was received.
    kConfirmation,
    /// confirmation broadcast was received.
    kConfirmationBroadcast,
    /// response was received wait for expected time
    kReceivedResponse,
    /// transaction was processed.
    kProcessed,
    /// timeout expired.
    kTimeout,
    /// transaction forced to abort.
    kAborted,
    /// error state during sending via iqrf interface.
    kError,
    /// iqrf interface queue is full
    kQueueFull
  };

private:
  /// Default timeout
  static const int DEFAULT_TIMEOUT = 200;
  /// Minimal timeout used if required by user is too low
  static const int MINIMAL_TIMEOUT = 200;
  /// Zero value used to indicate infinit timeout in special cases (discovery)
  static const int INFINITE_TIMEOUT = 0;
  /// An extra timeout added to timeout from a confirmation packet.
  static const int32_t SAFETY_TIMEOUT_MS = 40;

  /// Result object tobe returned when the transaction finishes
  std::unique_ptr<DpaTransactionResult2> m_dpaTransactionResultPtr;

  /// future/promise pair bounding the transaction life (can be used just once)
  std::future<int> m_future;
  std::promise<int> m_promise;

  /// functor to send the request message towards the coordinator
  SendDpaMessageFunc m_sender;

  /// condition used to wait on get()
  std::mutex m_getConditionVariableMutex;
  std::condition_variable m_getConditionVariable;

  /// condition used to wait for confirmation and response messages from coordinator
  std::mutex m_conditionVariableMutex;
  std::condition_variable m_conditionVariable;

  /// actual communication mode
  IDpaHandler2::RfMode m_currentCommunicationMode;

  //TODO these values shall be protected to be thread safe
  std::chrono::system_clock::time_point m_startTime;
  uint32_t m_userTimeoutMs = DEFAULT_TIMEOUT; //required by user
  uint32_t m_expectedDurationMs = INFINITE_TIMEOUT;

  /// transaction state
  std::atomic<DpaTransfer2State> m_state = kCreated;

  /// iqrf structure info to estimate transaction processing time
  int8_t m_hops;
  int8_t m_timeslotLength;
  int8_t m_hopsResponse;

public:
  DpaTransaction2(const DpaMessage& request, IDpaHandler2::RfMode mode, int32_t timeout, SendDpaMessageFunc sender)
    :m_sender(sender)
    ,m_dpaTransactionResultPtr(new DpaTransactionResult2(request))
    ,m_currentCommunicationMode(mode)
  {
    // init future object
    m_future = m_promise.get_future();

    const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();
    int32_t requiredTimeout = timeout;

    // check and correct timeout here before blocking:
    if (requiredTimeout < 0) {
      // default timeout
      timeout = DEFAULT_TIMEOUT;
    }
    else if (requiredTimeout == INFINITE_TIMEOUT) {
      // it is allowed just for Coordinator Discovery
      if (message.DpaPacket().DpaRequestPacket_t.NADR != COORDINATOR_ADDRESS ||
        message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_COORDINATOR_DISCOVERY) {
        // force setting minimal timing as only Discovery can have infinite timeout
        TRC_WAR("User: " << PAR(requiredTimeout) << " forced to: " << PAR(MINIMAL_TIMEOUT));
        requiredTimeout = MINIMAL_TIMEOUT;
      }
      else {
        TRC_WAR(PAR(requiredTimeout) << " allowed for DISCOVERY message");
      }
    }
    else if (requiredTimeout < MINIMAL_TIMEOUT) {
      TRC_WAR("User: " << PAR(requiredTimeout) << " forced to: " << PAR(MINIMAL_TIMEOUT));
      requiredTimeout = MINIMAL_TIMEOUT;
    }
    m_userTimeoutMs = requiredTimeout; // checked and corrected timeout 

  }

  virtual ~DpaTransaction2()
  {
  }

  // blocking function called from user code
  // user calls:
  // std::shared_ptr<IDpaTransaction2> dt = m_dpa->executeDpaTransaction(dpaTask->getRequest());
  // std::unique_ptr<IDpaTransactionResult2> dtr = dt->get(); //wait for async result
  std::unique_ptr<IDpaTransactionResult2> get0()
  {
    // error code to be stored in result
    int errorCode = 0;

    if (m_userTimeoutMs == INFINITE_TIMEOUT) {
      // infinite timeout blocks until the result becomes available
      m_future.wait();
      errorCode = m_future.get();
    }
    else {
      // TODO it shall be handle differently if estimation is longer then user time
      if (m_future.wait_for(std::chrono::milliseconds(m_userTimeoutMs)) == std::future_status::timeout) {
        //TODO handle iqrf BUSY or queue full
        // future timeout 
        TRC_WAR("Transaction timeout.");
        errorCode = IDpaTransactionResult2::TRN_ERROR_IFACE_BUSY;
      }
      else {
        errorCode = m_future.get();
      }
    }
    m_dpaTransactionResultPtr->setErrorCode(errorCode); // update error code in result

    // return result and move ownership 
    std::unique_ptr<IDpaTransactionResult2> result = std::move(m_dpaTransactionResultPtr);
    return result;
  }

  // blocking function called from user code
  // user calls:
  // std::shared_ptr<IDpaTransaction2> dt = m_dpa->executeDpaTransaction(dpaTask->getRequest());
  // std::unique_ptr<IDpaTransactionResult2> dtr = dt->get(); //wait for async result
  std::unique_ptr<IDpaTransactionResult2> get()
  {
    // wait for transaction start
    bool waitResult;
    {
      std::unique_lock<std::mutex> lck(m_getConditionVariableMutex);
      waitResult = m_getConditionVariable.wait_for(lck, std::chrono::milliseconds(m_userTimeoutMs), [&] { return m_state == kCreated; });
    }
    if (waitResult) {
      // finished waiting for start now it is controlled by execute()
      std::unique_lock<std::mutex> lck(m_getConditionVariableMutex);
      m_getConditionVariable.wait_for(lck, std::chrono::milliseconds(m_userTimeoutMs), [&] {
        switch (m_state) {
        case kCreated:
        case kSent:
        case kSentCoordinator:
        case kConfirmation:
        case kConfirmationBroadcast:
        case kReceivedResponse:
          return true;
        case kProcessed:
        case kTimeout:
        case kAborted:
        case kError:
        case kQueueFull:
        default:
          return false;
        }
      });
    }
    else {
      //timeout
      TRC_WAR("Transaction timeout - transaction was not started in time.");
      m_dpaTransactionResultPtr->setErrorCode(IDpaTransactionResult2::TRN_ERROR_IFACE_BUSY);
    }

    // return result and move ownership 
    std::unique_ptr<IDpaTransactionResult2> result = std::move(m_dpaTransactionResultPtr);
    return result;
  }

  // Transaction is started via this method. It is called from DpaHandler queue worker thread
  // If the function is not invoked in time required by user, the transaction future expires
  // and this situation is reported via transaction result error code (coordinator busy or queue full)
  void execute(bool queued)
  {
    const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();
    int errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;

    if (queued) {
      // init transaction state
      if (message.NodeAddress() == COORDINATOR_ADDRESS) {
        m_state = kSentCoordinator;
      }
      else {
        m_state = kSent;
      }

      // send message toward coordinator via send functor
      try {
        m_sender(message);
      }
      catch (std::exception& e) {
        TRC_WAR("Send error occured: " << e.what());
        m_state = kError;
      }

      // start transaction timer
      m_startTime = std::chrono::system_clock::now();
    }
    else {
      //transaction is not handled 
      m_state = kQueueFull;
    }

    // 1st notification to get() 
    {
      std::unique_lock<std::mutex> lck(m_getConditionVariableMutex);
      m_getConditionVariable.notify_one();
      // now get() waits for ever and depends on next processing
    }

    int32_t remains = 0;
    bool expired = false;

    do {
      //int32_t remains = 0;

      if (INFINITE_TIMEOUT != m_userTimeoutMs && m_expectedDurationMs > 0) {
        // passed time from sent request
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_startTime);
        remains = m_expectedDurationMs - duration.count();
        expired = remains <= 0;
      }

      switch (m_state)
      {
      case kSent:
      case kSentCoordinator:
      case kConfirmation:
        if (expired) {
          m_state = kTimeout;
          errorCode = DpaTransactionResult2::TRN_ERROR_TIMEOUT;
        }
        break;
      case kConfirmationBroadcast:
        if (expired) {
          m_state = kProcessed;
          errorCode = STATUS_NO_ERROR;
        }
        break;
      case kReceivedResponse:
        if (expired) {
          m_state = kProcessed;
          errorCode = STATUS_NO_ERROR;
        }
        break;
      case kProcessed:
        errorCode = STATUS_NO_ERROR;
        break;
      case kTimeout:
        errorCode = DpaTransactionResult2::TRN_ERROR_TIMEOUT;
        break;
      case kAborted:
        errorCode = DpaTransactionResult2::TRN_ERROR_ABORTED;
        break;
      case kError:
        errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;
        break;
      case kQueueFull:
        errorCode = IDpaTransactionResult2::TRN_ERROR_IFACE_QUEUE_FULL;
        break;
      default:
        errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;
      }

      // transaction doesn't finish yet so wait for remaining expected time
      if (!expired) {
        std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
        m_conditionVariable.wait_for(lck, std::chrono::milliseconds(remains));
      }

    } while (!expired);

    // update error code in result
    m_dpaTransactionResultPtr->setErrorCode(errorCode);

    // 2st notification to get() 
    {
      std::unique_lock<std::mutex> lck(m_getConditionVariableMutex);
      m_getConditionVariable.notify_one();
      // now get() when notified returns
    }

    // sync with future
    //m_promise.set_value(errorCode);
  }

  void ProcessReceivedMessage(const DpaMessage& receivedMessage)
  {
    TRC_ENTER("");
    // direction
    DpaMessage::MessageType messageDirection = receivedMessage.MessageDirection();
    int32_t estimatedTimeMs = 0;

    //check transaction state
    switch (m_state)
    {
    case kSent:
    case kSentCoordinator:
    case kConfirmation:
    case kConfirmationBroadcast:
    case kReceivedResponse:
      break; //continue
    case kCreated:
    case kProcessed:
    case kTimeout:
    case kAborted:
    case kError:
    default:
      return;
    }

    //check massage validity
    // no request is expected
    if (messageDirection != DpaMessage::kResponse && messageDirection != DpaMessage::kConfirmation) {
      throw unexpected_packet_type("Response is expected.");
    }
    const DpaMessage& request = m_dpaTransactionResultPtr->getRequest();
    // same as sent request
    if (receivedMessage.NodeAddress() != request.NodeAddress()) {
      throw unexpected_peripheral("Different node address than in sent message.");
    }
    // same as sent request
    if (receivedMessage.PeripheralType() != request.PeripheralType()) {
      throw unexpected_peripheral("Different peripheral type than in sent message.");
    }
    // same as sent request
    if ((receivedMessage.PeripheralCommand() & ~0x80) != request.PeripheralCommand()) {
      throw unexpected_command("Different peripheral command than in sent message.");
    }

    // process confirmation
    if (messageDirection == DpaMessage::kConfirmation) {
      if (receivedMessage.NodeAddress() == DpaMessage::kBroadCastAddress) {
        m_state = kConfirmationBroadcast;
      }
      else {
        m_state = kConfirmation;
      }

      // setting timeout based on the confirmation
      TIFaceConfirmation iFace = receivedMessage.DpaPacket().DpaResponsePacket_t.DpaMessage.IFaceConfirmation;

      // save for later use with response
      m_hops = iFace.Hops;
      m_timeslotLength = iFace.TimeSlotLength;
      m_hopsResponse = iFace.HopsResponse;

      if (m_currentCommunicationMode == IDpaHandler2::RfMode::kLp) {
        estimatedTimeMs = EstimateLpTimeout(m_hops, m_timeslotLength, m_hopsResponse);
      }
      else { // std
        estimatedTimeMs = EstimateStdTimeout(m_hops, m_timeslotLength, m_hopsResponse);
      }

      m_dpaTransactionResultPtr->setConfirmation(receivedMessage);
      TRC_INF("Confirmation processed.");
    }

    // process response
    else {
      // if there was a request to coordinator then after receiving response it is allowed to send another
      if (m_state == kSentCoordinator) {
        // done, next request gets ready 
        m_state = kProcessed;
      }
      else {
        // only if there is not infinite timeout
        if (m_expectedDurationMs != 0) {
          m_state = kReceivedResponse; 
          /////////////////////////////
          // TODO is it necessary here to wait if we have the response already?
          // is it aditional wait time for some reason depending on response len?
          if (m_currentCommunicationMode == IDpaHandler2::RfMode::kLp) {
            estimatedTimeMs = EstimateLpTimeout(m_hops, m_timeslotLength, m_hopsResponse,
              receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2));
          }
          else { // std
            estimatedTimeMs = EstimateStdTimeout(m_hops, m_timeslotLength, m_hopsResponse,
              receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2));
          }
          /////////////////////////////
          m_expectedDurationMs = estimatedTimeMs;
        }
        // infinite timeout
        else {
          // done, next request gets ready 
          m_state = kProcessed;
        }
      }

      m_dpaTransactionResultPtr->setResponse(receivedMessage);
      TRC_INF("Response processed.");
    }

    // estimation done
    if (estimatedTimeMs >= 0) {
      // either default timeout is 200 or user sets lower time than estimated
      if (m_userTimeoutMs < estimatedTimeMs) {
        // in both cases use estimation from confirmation
        m_userTimeoutMs = estimatedTimeMs;
      }
      // set new duration
      // there is also case when user sets higher than estimation then user choice is set
      m_expectedDurationMs = m_userTimeoutMs;
      TRC_INF("Expected duration to wait :" << PAR(m_expectedDurationMs));
    }

    // start time when dpa request is sent and rerun again when confirmation is received
    m_startTime = std::chrono::system_clock::now();

    // notification to execute() 
    {
      std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
      m_conditionVariable.notify_one();
    }

    TRC_LEAVE("");
  }

private:
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

    estimatedTimeoutMs += (hopsResponse + 1) * responseTimeSlotLengthMs + SAFETY_TIMEOUT_MS;

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

    estimatedTimeoutMs += (hopsResponse + 1) * responseTimeSlotLengthMs + SAFETY_TIMEOUT_MS;

    TRC_INF("Estimated LP timeout: " << PAR(estimatedTimeoutMs));
    TRC_LEAVE("");
    return estimatedTimeoutMs;
  }

  void Abort() {
    m_state = kAborted;
  }
};

/////////////////////////////////////
// class DpaHandler2::Imp
/////////////////////////////////////
class DpaHandler2::Imp
{
public:
  // maximal queue lenght - can be prolonged if it make sense
  static const int QUEUE_MAX_LEN = 32;

  Imp(IChannel* iqrfInterface)
    :m_iqrfInterface(iqrfInterface)
  {
    m_dpaTransactionQueue = new TaskQueue<std::shared_ptr<DpaTransaction2>>([&](std::shared_ptr<DpaTransaction2> ptr) {
      m_pendingTransaction = ptr;
      size_t size = m_dpaTransactionQueue->size();
      if (size < QUEUE_MAX_LEN) {
        m_pendingTransaction->execute(true); // succesfully queued
      }
      else {
        TRC_ERR("Transaction queue overload: " << PAR(size));
        m_pendingTransaction->execute(false);  // queue full transaction not handled, error reported
      }
    });

    if (iqrfInterface == nullptr) {
      throw std::invalid_argument("DPA interface argument can not be nullptr.");
    }
    m_iqrfInterface = iqrfInterface;

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

    TRC_DBG(">>>>>>>>>>>>>>>>>>" << std::endl <<
      "Received from IQRF interface: " << std::endl << FORM_HEX(message.data(), message.length()));

    // incomming message
    DpaMessage receivedMessage;
    try {
      receivedMessage.FillFromResponse(message.data(), message.length());
    }
    catch (std::exception& e) {
      CATCH_EX("in processing msg", std::exception, e);
      return;
    }

    auto messageDirection = receivedMessage.MessageDirection();
    if (messageDirection == DpaMessage::MessageType::kRequest) {
      //Always Async
      processAsynchronousMessage(message);
      return;
    }
    else if (messageDirection == DpaMessage::MessageType::kResponse &&
      receivedMessage.DpaPacket().DpaResponsePacket_t.ResponseCode & STATUS_ASYNC_RESPONSE) {
      // async msg
      processAsynchronousMessage(message);
      return;
    }
    else {
      try {
        m_pendingTransaction->ProcessReceivedMessage(message);
      }
      catch (std::logic_error& le) {
        CATCH_EX("Process received message error...", std::logic_error, le);
      }
    }
  }

  std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request, int32_t timeout)
  {
    std::shared_ptr<DpaTransaction2> ptr(new DpaTransaction2(request, m_rfMode, timeout,
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

std::shared_ptr<IDpaTransaction2> DpaHandler2::executeDpaTransaction(const DpaMessage& request, int32_t timeout)
{
  return m_imp->executeDpaTransaction(request, timeout);
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
