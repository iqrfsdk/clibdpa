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
  if (m_iqrfInterface == nullptr) {
    throw std::invalid_argument("DPA interface argument can not be nullptr.");
  }
  m_iqrfInterface = iqrfInterface;

  // default timeout -1
  m_defaultTimeoutMs = kDefaultTimeout;

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
  // get ready new holder by setting
  m_currentTransfer->DefaultTimeout(m_defaultTimeoutMs);
  m_currentTransfer->ProcessSentMessage(message);
}

// transfer with transaction
DpaTransfer* DpaHandler::CreateDpaTransfer(DpaTransaction* dpaTransaction) const
{
  DpaTransfer* response;
  response = new DpaTransfer(dpaTransaction);

  if (m_currentCommunicationMode == kLp) {
    response->SetIqrfRfMode(DpaTransfer::kLp);
  }
  else {
    response->SetIqrfRfMode(DpaTransfer::kStd);
  }

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

  TRC_DBG(">>>>>>>>>>>>>>>>>>" << std::endl <<
    "Received from IQRF interface: " << std::endl << FORM_HEX(message.data(), message.length()));

  // new message
  DpaMessage receivedMessage;
  try {
    receivedMessage.FillFromResponse(message.data(), message.length());
  }
  catch (std::exception& e) {
    CATCH_EX("in processing msg", std::exception, e);
    return;
  }

  if (!ProcessMessage(receivedMessage)) {
    ProcessAsynchronousMessage(receivedMessage);
  }

  // notification about reception
  {
    std::unique_lock<std::mutex> lck(m_conditionVariableMutex);
    m_conditionVariable.notify_one();
  }
}

bool DpaHandler::ProcessMessage(const DpaMessage& message) {
  try {
    m_currentTransfer->ProcessReceivedMessage(message);
  }
  catch (std::logic_error& le) {
    return false;
  }
  return true;
}

DpaTransfer::DpaTransferStatus DpaHandler::Status() const {
  return m_currentTransfer->ProcessStatus();
}

void DpaHandler::RegisterAsyncMessageHandler(std::function<void(const DpaMessage&)> messageHandler) {
  m_asyncMessageMutex.lock();

  m_asyncMessageHandler = messageHandler;

  m_asyncMessageMutex.unlock();
}

void DpaHandler::ProcessAsynchronousMessage(DpaMessage& message) {
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

DpaHandler::IqrfRfCommunicationMode DpaHandler::GetRfCommunicationMode() const
{
  return m_currentCommunicationMode;
}

void DpaHandler::SetRfCommunicationMode(IqrfRfCommunicationMode mode)
{
  if (IsDpaMessageInProgress())
  {
    //TODO: add exception
  }

  m_currentCommunicationMode = mode;
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
  if (requiredTimeout > 0)
    Timeout(requiredTimeout);

  // update transfer state
  DpaTransfer::DpaTransferStatus status(DpaTransfer::kCreated);

  try {
    SendDpaMessage(message, &dpaTransaction);

    while (IsDpaTransactionInProgress(remains)) {
      //wait for remaining time
      {
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

  TRC_LEAVE("");
}
