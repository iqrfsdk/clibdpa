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
#include "IChannel.h"

#include "unexpected_command.h"
#include "unexpected_packet_type.h"
#include "unexpected_peripheral.h"

#include <future>

// Timing constants

/// Default timeout
static const int DEFAULT_TIMEOUT = 500;
/// Minimal timeout used if required by user is too low
static const int MINIMAL_TIMEOUT = 200;
/// Zero value used to indicate infinit timeout in special cases (discovery)
static const int INFINITE_TIMEOUT = 0;
/// An extra timeout added to timeout from a confirmation packet.
static const int32_t SAFETY_TIMEOUT_MS = 40;

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
  int m_errorCode = TRN_ERROR_ABORTED;
  /// response code
  int m_responseCode = TRN_OK;
  /// received and set response flag
  bool m_isResponded = false;
  /// received and set confirmation flag
  int m_isConfirmed = false;

public:
  DpaTransactionResult2() = delete;
  DpaTransactionResult2(const DpaMessage& request)
  {
    m_request_ts = std::chrono::system_clock::now();
    m_request = request;
  }

  int getErrorCode() const override
  {
    if (m_errorCode != TRN_OK) {
      return m_errorCode;
    }
    else {
      return m_responseCode;
    }
  }

  void overrideErrorCode(IDpaTransactionResult2::ErrorCode err) override
  {
    m_errorCode = err;
  }

  std::string getErrorString() const override
  {
    switch (m_errorCode) {

    case TRN_ERROR_BAD_RESPONSE:
      return "BAD_RESPONSE";
    case TRN_ERROR_BAD_REQUEST:
      return "BAD_REQUEST";
    case TRN_ERROR_IFACE_BUSY:
      return "ERROR_IFACE_BUSY";
    case TRN_ERROR_IFACE:
      return "ERROR_IFACE";
    case TRN_ERROR_ABORTED:
      return "ERROR_ABORTED";
    case TRN_ERROR_IFACE_QUEUE_FULL:
      return "ERROR_IFACE_QUEUE_FULL";
    case TRN_ERROR_TIMEOUT:
      return "ERROR_TIMEOUT";
    case TRN_OK:
      return "ok";
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

  bool isConfirmed() const override
  {
    return m_isConfirmed;
  }

  bool isResponded() const override
  {
    return m_isResponded;
  }

  void setConfirmation(const DpaMessage& confirmation)
  {
    m_confirmation_ts = std::chrono::system_clock::now();
    m_confirmation = confirmation;
    m_isConfirmed = true;
  }

  void setResponse(const DpaMessage& response)
  {
    m_response_ts = std::chrono::system_clock::now();
    m_response = response;
    if (0 < response.GetLength()) {
      m_responseCode = response.DpaPacket().DpaResponsePacket_t.ResponseCode;
      m_isResponded = true;
    }
    else {
      m_isResponded = false;
    }
  }

  void setErrorCode(int errorCode)
  {
    if (errorCode == TRN_OK && m_responseCode != TRN_OK) {
      errorCode = m_responseCode;
    }
    m_errorCode = errorCode;
  }
};

/////////////////////////////////////
// class DpaTransaction2
/////////////////////////////////////

//This is key timing class. Its methods execute(), get(), processReceivedMessage() runs concurently and has to be synchronized
//
// user calls:
// std::shared_ptr<IDpaTransaction2> dt = m_dpa->executeDpaTransaction(dpaTask->getRequest());
// std::unique_ptr<IDpaTransactionResult2> dtr = dt->get();
//
// When user invokes m_dpa->executeDpaTransaction() this class is constructed and stored by m_dpa to the queue of transactions.
// If there is no pending transaction the first one from the queue is execute() by DpaHandler queue worker thread.
// If the function is not invoked in time required by user, the transaction timer expires and 
// this situation is reported via transaction result error code (coordinator busy or queue full)
//
// execute() gets to point waiting for confirmation or response from coordinator. These messages are handled by processReceivedMessage().
// The function computes expected time set appropriate transaction state and pass controll back to execute() to continue or finish the transaction.
// When transaction is finish the function execute() pass control to get() and it returns transaction result to user.
//

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
  /// Result object tobe returned when the transaction finishes
  std::unique_ptr<DpaTransactionResult2> m_dpaTransactionResultPtr;

  /// transaction state
  DpaTransfer2State m_state = kCreated;
  /// signalize final state
  bool m_finish = false;

  /// actual communication mode
  IDpaHandler2::RfMode m_currentCommunicationMode;

  /// functor to send the request message towards the coordinator
  SendDpaMessageFunc m_sender;

  uint32_t m_userTimeoutMs = DEFAULT_TIMEOUT; //required by user
  uint32_t m_expectedDurationMs = DEFAULT_TIMEOUT;
  bool m_infinitTimeout = false;

  /// iqrf structure info to estimate transaction processing time
  int8_t m_hops = 0;
  int8_t m_timeslotLength = 0;
  int8_t m_hopsResponse = 0;

  /// condition used to wait for confirmation and response messages from coordinator
  std::condition_variable m_conditionVariable;
  /// mutex to protect shared variables controlled by condition variable
  std::mutex m_conditionVariableMutex;

  /// internal transaction ID
  uint32_t m_transactionId;

public:
  DpaTransaction2() = delete;

  DpaTransaction2(const DpaMessage& request, IDpaHandler2::RfMode mode, int32_t defaultTimeout, int32_t userTimeout, SendDpaMessageFunc sender)
    :m_sender(sender)
    ,m_dpaTransactionResultPtr(new DpaTransactionResult2(request))
    ,m_currentCommunicationMode(mode)
  {
    TRC_ENTER(PAR(mode) << PAR(defaultTimeout) << PAR(userTimeout))
    static uint32_t transactionId = 0;
    m_transactionId = ++transactionId;

    const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();
    
    if (userTimeout < 0) {
      userTimeout = defaultTimeout;
    }
    
    int32_t requiredTimeout = userTimeout;

    // check and correct timeout here before blocking:
    if (requiredTimeout < 0) {
      // default timeout
      requiredTimeout = DEFAULT_TIMEOUT;
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
        TRC_WAR(PAR(requiredTimeout) << " infinite timeout allowed for DISCOVERY message");
        requiredTimeout = DEFAULT_TIMEOUT;
        m_infinitTimeout = true;
      }
    }
    else if (requiredTimeout < MINIMAL_TIMEOUT) {
      TRC_WAR("User: " << PAR(requiredTimeout) << " forced to: " << PAR(MINIMAL_TIMEOUT));
      requiredTimeout = MINIMAL_TIMEOUT;
    }
    m_userTimeoutMs = requiredTimeout; // checked and corrected timeout 
    TRC_LEAVE("Using: " << PAR(m_userTimeoutMs));
  }

  virtual ~DpaTransaction2()
  {
  }

  void abort() override {
    std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
    m_state = kAborted;
    m_conditionVariable.notify_all();
  }

  //-----------------------------------------------------
  std::unique_ptr<IDpaTransactionResult2> get()
  {
    // wait for transaction start
    TRC_DBG("wait for start: " << PAR(m_transactionId));
    
    // lock this function except blocking in wait_for()
    std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
    
    while (m_infinitTimeout) {
      // wait_for() unlock lck when blocking and lock it again when get out, waiting continue if not started (predicate == false)
      if (!m_conditionVariable.wait_for(lck, std::chrono::milliseconds(m_userTimeoutMs), [&] { return m_state != kCreated; }))
      {
        // out of wait timeout
        if (!m_infinitTimeout) {
          TRC_WAR("Transaction timeout - transaction was not started in time.");
          m_dpaTransactionResultPtr->setErrorCode(IDpaTransactionResult2::TRN_ERROR_IFACE_BUSY);
          // return result and move ownership 
          return std::move(m_dpaTransactionResultPtr);
        }
        else {
          TRC_WAR("Infinit timeout - wait forever.");
        }
      }
      // out of wait notify from execute() or processReceivedMessage
      break;
    }

    // transaction started
    TRC_DBG("Started, wait for finish: " << PAR(m_transactionId));
    // wait_for() unlock lck when blocking and lock it again when get out, waiting continue if not finished (predicate == false)
    while (!m_conditionVariable.wait_for(lck, std::chrono::milliseconds(m_userTimeoutMs), [&] { return m_finish; }));

    // return result and move ownership 
    TRC_DBG("Finished: " << PAR(m_transactionId) << PAR(m_state));
    return std::move(m_dpaTransactionResultPtr);
  }

  //-----------------------------------------------------
  void execute(bool queued)
  {
    // lock this function except blocking in wait_for()
    std::unique_lock<std::mutex> lck(m_conditionVariableMutex);

    const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();

    if (queued) {
      // init transaction state
      if (message.NodeAddress() == COORDINATOR_ADDRESS) {
        m_state = kSentCoordinator;
      }
      else {
        m_state = kSent;
      }

      // init expected duration - no estimation yet, so use user timeout
      m_expectedDurationMs = m_userTimeoutMs;

      // send request toward coordinator via send functor
      try {
        m_sender(message);
        // now we can expect handling in processReceivedMessage()
      }
      catch (std::exception& e) {
        TRC_WAR("Send error occured: " << e.what());
        // init expected duration - we have final error state - just finish transaction
        m_expectedDurationMs = 0;
        m_state = kError;
      }
    }
    else {
      // transaction is not handled 
      m_state = kQueueFull;
      // init expected duration - we have final error state - just finish transaction
      m_expectedDurationMs = 0;
    }

    // 1st notification to get() - we started transaction 
    m_conditionVariable.notify_one();

    int errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;
    bool finish = true;
    bool expired = false;

    do {

      finish = true; // end this cycle
      expired = false;

      // wait on conditon 
      if (m_expectedDurationMs > 0) {
        // wait_for() unlock lck when blocking and lock it again when get out, processReceivedMessage() is able to do its job as it can lock now
        if (std::cv_status::timeout == m_conditionVariable.wait_for(lck, std::chrono::milliseconds(m_expectedDurationMs))) {
          // out of wait on timeout
          expired = true;
        }
        // out of wait on notify from processReceivedMessage()
      }

      // evaluate state
      switch (m_state)
      {
      case kSent:
      case kSentCoordinator:
      case kConfirmation:
        if (expired) {
          if (!m_infinitTimeout) {
            m_state = kTimeout;
            errorCode = DpaTransactionResult2::TRN_ERROR_TIMEOUT;
          }
          else {
            // reset finish we have to wait forever
            finish = false;
          }
        }
        else {
          // reset finish we didn't finish yet
          finish = false;
        }
        break;
      case kConfirmationBroadcast:
        if (expired) {
          m_state = kProcessed;
          errorCode = DpaTransactionResult2::TRN_OK;
        }
        else {
          // reset finish we didn't finish yet
          finish = false;
        }
        break;
      case kReceivedResponse:
        if (expired) {
          m_state = kProcessed;
          errorCode = DpaTransactionResult2::TRN_OK;
        }
        else {
          // reset finish we didn't finish yet
          finish = false;
        }
        break;
      case kProcessed:
        errorCode = DpaTransactionResult2::TRN_OK;
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

    } while (!finish);

    // update error code in result
    m_dpaTransactionResultPtr->setErrorCode(errorCode);

    // signalize final state
    m_finish = true;

    // 2st notification to get() 
    m_conditionVariable.notify_one();
  }

  //-----------------------------------------------------
  void processReceivedMessage(const DpaMessage& receivedMessage)
  {
    TRC_ENTER("");

    // lock this function
    std::unique_lock<std::mutex> lck(m_conditionVariableMutex);

    //check transaction state
    if (m_finish) {
      return; //nothing to do, just double check
    }

    DpaMessage::MessageType messageDirection = receivedMessage.MessageDirection();

    //check massage validity
    // TODO verify exception handling. Do we need the exception here at all?
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

    int32_t estimatedTimeMs = 0;

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
      
      if (estimatedTimeMs > 0) {
        TRC_INF("Expected duration to wait :" << PAR(m_userTimeoutMs) << PAR(estimatedTimeMs));
        if (estimatedTimeMs >= m_userTimeoutMs) {
          m_expectedDurationMs = estimatedTimeMs;
        }
        else {
          // user wants to wait more then estimated so keep the wish
          m_expectedDurationMs = m_userTimeoutMs;
        }
      }

      TRC_DBG("From confirmation: " << PAR(estimatedTimeMs));

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
        if (!m_infinitTimeout) {
          m_state = kReceivedResponse;
          /////////////////////////////
          // TODO is it necessary here to wait if we have the response already?
          // or is it aditional refresh timeout for some reason depending on response len?
          if (m_currentCommunicationMode == IDpaHandler2::RfMode::kLp) {
            estimatedTimeMs = EstimateLpTimeout(m_hops, m_timeslotLength, m_hopsResponse,
              receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2));
          }
          else { // std
            estimatedTimeMs = EstimateStdTimeout(m_hops, m_timeslotLength, m_hopsResponse,
              receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2));
          }
          TRC_DBG("From response: " << PAR(estimatedTimeMs));
          m_expectedDurationMs = estimatedTimeMs;
          if (m_expectedDurationMs <= 0) {
            m_state = kProcessed;
          }
          /////////////////////////////
        }
        // infinite timeout
        else {
          // done, next request gets ready 
          m_state = kProcessed; // TODO If the refresh timeout is necessary would be here as well?
        }
      }

      m_dpaTransactionResultPtr->setResponse(receivedMessage);
      TRC_INF("Response processed.");
    }

    // notification to execute() and get()
    m_conditionVariable.notify_all();

    TRC_LEAVE("");
  }

private:
  // TODO it is not necessary pass the values as they are stored in members
  // m_hops, m_timeslotLength, m_hopsResponse,
  // we will need other network structure info for FRC evaluation
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

};

/////////////////////////////////////
// class DpaHandler2::Imp
/////////////////////////////////////
class DpaHandler2::Imp
{
public:
  // maximal queue lenght - can be prolonged if it make sense
  static const int QUEUE_MAX_LEN = 16;

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
    // kill DpaTransaction if any
    if (m_pendingTransaction) {
      m_pendingTransaction->abort();
    }
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
        m_pendingTransaction->processReceivedMessage(message);
      }
      catch (std::logic_error& le) {
        CATCH_EX("Process received message error...", std::logic_error, le);
      }
    }
  }

  std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request, int32_t timeout)
  {
    if (request.GetLength() <= 0) {
      TRC_WAR("Empty request => nothing to sent and transaction aborted");
      std::shared_ptr<DpaTransaction2> ptr(new DpaTransaction2(request, m_rfMode, m_defaultTimeout, timeout, nullptr));
      return ptr;
    }
    std::shared_ptr<DpaTransaction2> ptr(new DpaTransaction2(request, m_rfMode, m_defaultTimeout, timeout,
      [&](const DpaMessage& r) {
        sendRequest(r);
      }
    ));
    m_dpaTransactionQueue->pushToQueue(ptr);
    return ptr;
  }

  int getTimeout() const
  {
    return m_defaultTimeout;
  }

  void setTimeout(int timeout)
  {
    m_defaultTimeout = timeout;
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
  int m_defaultTimeout = DEFAULT_TIMEOUT;

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
