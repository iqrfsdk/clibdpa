#include "include/DpaMessage.h"
#include "include/DpaHandler.h"
#include "IqrfLogging.h"

DpaHandler::DpaHandler(IChannel* dpa_interface) {
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

void DpaHandler::SendDpaMessage(const DpaMessage& message, DpaTransaction* responseHndl) {
  if (IsDpaMessageInProgress()) {
    throw std::logic_error("Other Dpa Message is in progress.");
  }

  try {
    dpa_interface_->sendTo(std::basic_string<unsigned char>(message.DpaPacketData(), message.Length()));
  }
  catch (std::exception& e) {
    throw std::logic_error("Message was not send.");
  }

  delete current_request_;
  if (!responseHndl)
    current_request_ = new DpaRequest();
  else
    current_request_ = new DpaRequest(responseHndl);
  current_request_->DefaultTimeout(default_timeout_ms_);
  current_request_->ProcessSentMessage(message);
}

void DpaHandler::ExecuteDpaTransaction(DpaTransaction& dpaTransaction)
{
  const DpaMessage& message = dpaTransaction.getMessage();

  int32_t remains(0);
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
