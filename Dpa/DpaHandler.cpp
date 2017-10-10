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

#include "DpaHandler.h"
#include "DpaMessage.h"
#include "IqrfLogging.h"

DpaHandler::DpaHandler(IChannel* iqrfInterface) : m_currentCommunicationMode(kStd) {

  if (iqrfInterface == nullptr) {
    throw std::invalid_argument("DPA interface argument can not be nullptr.");
  }
  m_iqrfInterface = iqrfInterface;

  // default timeout 400ms
  m_defaultTimeoutMs = kDefaultTimeout;
  TRC_DBG("Ctor default user timeout: " << PAR(m_defaultTimeoutMs));

  // register callback for cdc or spi interface
  m_iqrfInterface->registerReceiveFromHandler([&](const std::basic_string<unsigned char>& msg) -> int {
    ResponseMessageHandler(msg);
    return 0;
  });

  m_currentTransfer = new DpaTransfer();
}

DpaHandler::~DpaHandler() {
  delete m_currentTransfer;
}

void DpaHandler::SendDpaMessage(const DpaMessage& message, DpaTransaction* responseHandler) {

  TRC_ENTER("");

  if (IsDpaMessageInProgress()) {
    throw std::logic_error("Other Dpa Message is in progress.");
  }

  try {
    TRC_DBG("<<<<<<<<<<<<<<<<<<" << std::endl <<
      "Sent to DPA interface: " << std::endl << FORM_HEX(message.DpaPacketData(), message.GetLength()));
    m_iqrfInterface->sendTo(std::basic_string<unsigned char>(message.DpaPacketData(), message.GetLength()));
  }
  catch (std::exception& e) {
    throw std::logic_error("Message was not send.");
  }

  // delete holder of the current dpa request - msg already sent
  delete m_currentTransfer;

  // create new holder for the reception
  m_currentTransfer = CreateDpaTransfer(responseHandler);
  
  // get ready new holder by setting user timeout if sets otherwise -1
  m_currentTransfer->DefaultTimeout(m_defaultTimeoutMs);
  TRC_DBG("Setting user timeout: " << PAR(m_defaultTimeoutMs));
  
  try {
    m_currentTransfer->ProcessSentMessage(message);
  }
  catch(std::logic_error& le) {
    CATCH_EX("while sending msg", std::logic_error, le);
  }
  
  TRC_LEAVE("");
}

// transfer with transaction
DpaTransfer* DpaHandler::CreateDpaTransfer(DpaTransaction* dpaTransaction) const
{
  DpaTransfer* response = new DpaTransfer(dpaTransaction, GetRfCommunicationMode());
  return response;
}

bool DpaHandler::IsDpaMessageInProgress() const {
  return m_currentTransfer->IsInProgress();
}

bool DpaHandler::IsDpaTransactionInProgress(int32_t& expectedDuration) const {
  return m_currentTransfer->IsInProgress(expectedDuration);
}

// any received message from the channel
void DpaHandler::ResponseMessageHandler(const std::basic_string<unsigned char>& message) {
  if (message.length() == 0)
    return;

  // signal that message is received, but not yet processed
  m_currentTransfer->MessageReceived(true);

  TRC_DBG(">>>>>>>>>>>>>>>>>>" << std::endl <<
    "Received from IQRF interface: " << std::endl << FORM_HEX(message.data(), message.length()));

  // new message
  DpaMessage receivedMessage;
  try {
    receivedMessage.FillFromResponse(message.data(), message.length());
  }
  catch (std::exception& e) {
    m_currentTransfer->MessageReceived(false);
    CATCH_EX("in processing msg", std::exception, e);
    return;
  }
  
  auto messageDirection = receivedMessage.MessageDirection();
  if (messageDirection == DpaMessage::MessageType::kRequest) {
    //Always Async
    m_currentTransfer->MessageReceived(false);
    ProcessAsynchronousMessage(message);
    return;
  }
  else if (messageDirection == DpaMessage::MessageType::kResponse && 
    receivedMessage.DpaPacket().DpaResponsePacket_t.ResponseCode & STATUS_ASYNC_RESPONSE) {
    // async msg
    m_currentTransfer->MessageReceived(false);
    ProcessAsynchronousMessage(message);
    return;
  }
  else {
    try {
      // transfer msg
      m_currentTransfer->ProcessReceivedMessage(message);
      // notification about reception
      {
        std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
        m_conditionVariable.notify_one();
        TRC_INF("Notify from ResponseMessageHandler: message received.");
      }
    }
    catch (std::logic_error& le) {
      CATCH_EX("Process received message error...", std::logic_error, le);
    }
  }
}

DpaTransfer::DpaTransferStatus DpaHandler::Status() const {
  return m_currentTransfer->ProcessStatus();
}

void DpaHandler::RegisterAsyncMessageHandler(std::function<void(const DpaMessage&)> messageHandler) {
  m_asyncMessageMutex.lock();

  m_asyncMessageHandler = messageHandler;

  m_asyncMessageMutex.unlock();
}

void DpaHandler::ProcessAsynchronousMessage(const DpaMessage& message) {
  m_asyncMessageMutex.lock();

  if (m_asyncMessageHandler) {
    m_asyncMessageHandler(message);
  }

  m_asyncMessageMutex.unlock();
}

void DpaHandler::UnregisterAsyncMessageHandler() {
  m_asyncMessageMutex.lock();

  m_asyncMessageHandler = nullptr;

  m_asyncMessageMutex.unlock();
}

const DpaTransfer& DpaHandler::CurrentTransfer() const {
  return *m_currentTransfer;
}

void DpaHandler::Timeout(int32_t timeoutMs) {
  m_defaultTimeoutMs = timeoutMs;
}

int32_t DpaHandler::Timeout() const {
  return m_defaultTimeoutMs;
}

IqrfRfCommunicationMode DpaHandler::GetRfCommunicationMode() const
{
  return m_currentCommunicationMode;
}

bool DpaHandler::SetRfCommunicationMode(IqrfRfCommunicationMode mode)
{
  if (IsDpaMessageInProgress())
  {
    TRC_ERR("Transfer in progress, RF mode has not been changed!");
    return false;
  }
  m_currentCommunicationMode = mode;
  return true;
}

// main execution 
void DpaHandler::ExecuteDpaTransaction(DpaTransaction& dpaTransaction)
{
  const DpaMessage& message = dpaTransaction.getMessage();

  int32_t remains(0);
  // dpa handler timeout
  int32_t defaultTimeout = Timeout();
  // dpa task timeout
  int32_t requiredTimeout = dpaTransaction.getTimeout();

  // update handler timeout from task
  if (requiredTimeout >= 0)
    Timeout(requiredTimeout);

  // update transfer state
  DpaTransfer::DpaTransferStatus status(DpaTransfer::kCreated);

  try {
    SendDpaMessage(message, &dpaTransaction);

    // waits till message processed - kProcessed
    while (IsDpaTransactionInProgress(remains)) {
      //waits for remaining time or notifing
      {
        if (remains > 0) {
          TRC_DBG("Conditional wait - time to wait yet: " << PAR(remains));
        }
        else 
          TRC_DBG("Conditional wait - time is out: " << PAR(remains));

        std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
        m_conditionVariable.wait_for(lck, std::chrono::milliseconds(remains));
      }
    }
    
    // update transfer state
    status = Status();
  }
  catch (std::exception& e) {
    TRC_WAR("Send error occured: " << e.what());
    status = DpaTransfer::DpaTransferStatus::kError;
  }

  Timeout(defaultTimeout);
  dpaTransaction.processFinish(status);
}

void DpaHandler::KillDpaTransaction()
{
  TRC_ENTER("");
  TRC_WAR("Killing transaction");

  //TODO
  m_currentTransfer->Abort();

  std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
  m_conditionVariable.notify_one();
  TRC_INF("Notify from KillDpaTransaction.");

  TRC_LEAVE("");
}
