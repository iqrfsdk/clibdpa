#include "include/DpaMessage.h"
#include "include/DpaHandler.h"

DpaHandler::DpaHandler(DpaInterface* dpa_interface) {
  if (dpa_interface == nullptr) {
    throw std::invalid_argument("dpa_interface argument can not be nullptr.");
  }
  default_timeout_ms_ = kDefaultTimeout;

  dpa_interface_ = dpa_interface;

  dpa_interface_->RegisterResponseHandler(
    std::bind(&DpaHandler::ResponseHandler, this, std::placeholders::_1, std::placeholders::_2));

  current_request_ = new DpaRequest();
}

DpaHandler::~DpaHandler() {
  delete current_request_;
}

void DpaHandler::ResponseHandler(unsigned char* data, uint32_t length) {
  if (data == nullptr)
    return;

  if (length == 0)
    return;

  DpaMessage received_message;
  try {
    received_message.FillFromResponse(data, length);
  }
  catch (...) {
    return;
  }
  if (!ProcessMessage(received_message)) {
    ProcessUnexpectedMessage(received_message);
  }
}

void DpaHandler::SendDpaMessage(const DpaMessage& message, IDpaResponseHandler* responseHndl) {
  if (IsDpaMessageInProgress())
    throw std::logic_error("Other Dpa Message is in progress.");

  auto response = dpa_interface_->SendRequest(message.DpaPacketData(), message.Length());
  if (response < 0)
    throw std::logic_error("Message was not send.");

  delete current_request_;
  if (!responseHndl)
    current_request_ = new DpaRequest();
  else
    current_request_ = new DpaRequest(responseHndl);
  current_request_->DefaultTimeout(default_timeout_ms_);
  current_request_->ProcessSentMessage(message);
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
