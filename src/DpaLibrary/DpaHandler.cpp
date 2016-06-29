#include <DpaMessage.h>
#include "include/DpaHandler.h"

DpaHandler::DpaHandler(DpaInterface* dpa_interface) {
  //TODO: Dopsat kontrolu argumentu
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

void DpaHandler::SendDpaMessage(DpaMessage& message) {
  if (IsDpaMessageInProgress())
	throw std::logic_error("Other Dpa Message is in progress.");

  auto response = dpa_interface_->SendRequest(message.DpaPacketData(),
											  message.Length());
  if (response < 0)
	throw std::logic_error("Message was not send.");

  delete current_request_;
  current_request_ = new DpaRequest();
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
