/**
 * Copyright 2015-2017 MICRORISC s.r.o.
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

#include "DpaMessage.h"
#include "DpaHandler.h"
#include "IqrfLogging.h"
#include "Dpa30xRequest.h"

DpaHandler::DpaHandler(IChannel* dpa_interface)
  : current_communication_mode_(kStd)
  , current_dpa_version_(k30x)
{
  if (dpa_interface == nullptr) {
    throw std::invalid_argument("dpa_interface argument can not be nullptr.");
  }
  default_timeout_ms_ = kDefaultTimeout;

  dpa_interface_ = dpa_interface;

  dpa_interface_->registerReceiveFromHandler([&](const std::basic_string<unsigned char>& msg) -> int {
    ResponseHandler(msg);
    return 0;
  });

  current_request_ = new DpaRequest();
}

DpaHandler::~DpaHandler() {
  delete current_request_;
}

void DpaHandler::ResponseHandler(const std::basic_string<unsigned char>& message) {
  if (message.length() == 0)
    return;

  TRC_DBG(">>>>>>>>>>>>>>>>>>" << std::endl <<
    "Received from DPA interface: " << std::endl << FORM_HEX(message.data(), message.length()));

  DpaMessage received_message;
  try {
    received_message.FillFromResponse(message.data(), message.length());
  }
  catch (std::exception& e) {
    CATCH_EX("in processing msg", std::exception, e);
    return;
  }
  if (!ProcessMessage(received_message)) {
    ProcessUnexpectedMessage(received_message);
  }
  {
    std::unique_lock<std::mutex> lck(condition_variable_mutex_);
    condition_variable_.notify_one();
  }
}

DpaRequest* DpaHandler::CreateDpaRequest(DpaTransaction* dpa_transaction) const
{
  DpaRequest* response;
  if (current_dpa_version_ == k22x)
  {
    response = new DpaRequest(dpa_transaction);
  }
  else
  {
    response = new Dpa30xRequest(dpa_transaction);
  }

  if (current_communication_mode_ == kLp)
  {
    response->IqrfRfMode(DpaRequest::kLp);
  }
  else
  {
    response->IqrfRfMode(DpaRequest::kStd);
  }

  return response;
}

void DpaHandler::SendDpaMessage(const DpaMessage& message, DpaTransaction* responseHndl) {
  if (IsDpaMessageInProgress()) {
    throw std::logic_error("Other Dpa Message is in progress.");
  }

  try {
    TRC_DBG("<<<<<<<<<<<<<<<<<<" << std::endl <<
      "Sent to DPA interface: " << std::endl << FORM_HEX(message.DpaPacketData(), message.Length()));

    dpa_interface_->sendTo(std::basic_string<unsigned char>(message.DpaPacketData(), message.Length()));
  }
  catch (std::exception& e) {
    throw std::logic_error("Message was not send.");
  }

  delete current_request_;

  current_request_ = CreateDpaRequest(responseHndl);

  current_request_->DefaultTimeout(default_timeout_ms_);
  current_request_->ProcessSentMessage(message);
}

void DpaHandler::ExecuteDpaTransaction(DpaTransaction& dpaTransaction)
{
  const DpaMessage& message = dpaTransaction.getMessage();

  int32_t remains(0);
  int32_t defaultTimeout = Timeout();
  int32_t requiredTimeout = dpaTransaction.getTimeout();

  if (requiredTimeout > 0)
    Timeout(requiredTimeout);

  DpaRequest::DpaRequestStatus status(DpaRequest::kCreated);

  try {
    SendDpaMessage(message, &dpaTransaction);

    while (IsDpaTransactionInProgress(remains)) {
      { //wait for remaining time
        std::unique_lock<std::mutex> lck(condition_variable_mutex_);
        condition_variable_.wait_for(lck, std::chrono::milliseconds(remains));
      }
    }
    status = Status();
  }
  catch (std::exception& e) {
    TRC_WAR("Send error occured: " << e.what());
  }

  Timeout(defaultTimeout);
  dpaTransaction.processFinish(status);
}

bool DpaHandler::ProcessMessage(const DpaMessage& message) {
  try {
    current_request_->ProcessReceivedMessage(message);
  }
  catch (std::logic_error& le) {
    return false;
  }
  return true;
}

bool DpaHandler::IsDpaMessageInProgress() const {
  return current_request_->IsInProgress();
}

bool DpaHandler::IsDpaTransactionInProgress(int32_t& expected_duration) const {
  return current_request_->IsInProgress(expected_duration);
}

DpaRequest::DpaRequestStatus DpaHandler::Status() const {
  return current_request_->Status();
}


void DpaHandler::UnregisterAsyncMessageHandler() {
  async_message_mutex_.lock();
  async_message_handler_ = nullptr;
  async_message_mutex_.unlock();
}

void DpaHandler::RegisterAsyncMessageHandler(std::function<void(const DpaMessage&)> message_handler) {
  async_message_mutex_.lock();
  async_message_handler_ = message_handler;
  async_message_mutex_.unlock();
}

void DpaHandler::ProcessUnexpectedMessage(DpaMessage& message) {
  async_message_mutex_.lock();
  if (async_message_handler_) {
    async_message_handler_(message);
  }
  async_message_mutex_.unlock();
}
const DpaRequest& DpaHandler::CurrentRequest() const {
  return *current_request_;
}

void DpaHandler::Timeout(int32_t timeout_ms) {
  default_timeout_ms_ = timeout_ms;
}

int32_t DpaHandler::Timeout() const {
  return default_timeout_ms_;
}

DpaHandler::DpaProtocolVersion DpaHandler::DpaVersion() const
{
  return current_dpa_version_;
}

void DpaHandler::DpaVersion(DpaProtocolVersion new_dpa_version)
{
  if (IsDpaMessageInProgress())
  {
    //TODO:  doplnit vyjimku
  }
  current_dpa_version_ = new_dpa_version;
}

DpaHandler::IqrfRfCommunicationMode DpaHandler::GetRfCommunicationMode() const
{
  return current_communication_mode_;
}

void DpaHandler::SetRfCommunicationMode(IqrfRfCommunicationMode mode)
{
  if (IsDpaMessageInProgress())
  {
    //TODO:  doplnit vyjimku
  }
  current_communication_mode_ = mode;
}
