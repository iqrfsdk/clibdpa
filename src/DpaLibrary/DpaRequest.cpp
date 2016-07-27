#include <iostream>
#include "unexpected_peripheral.h"
#include "DpaRequest.h"
#include "unexpected_packet_type.h"
#include "unexpected_command.h"

DpaRequest::DpaRequest()
	: status_(kCreated), sent_message_(nullptr), response_message_(nullptr), expected_duration_ms_(0),
	  timeout_ms_(-1) {
}

DpaRequest::~DpaRequest() {
  delete sent_message_;
  delete response_message_;
}

void DpaRequest::ProcessSentMessage(const DpaMessage& sent_message) {
  if (status_ != kCreated) {
	throw std::logic_error("Sent message already set.");
  }

  SetStatus(kSent);
  delete sent_message_;
  sent_message_ = new DpaMessage(sent_message);
  SetTimeoutForCurrentRequest();
}

void DpaRequest::ProcessConfirmationMessage(const DpaMessage& confirmation_packet) {
  status_ = kConfirmation;
  SetTimeoutForCurrentRequest(EstimatedTimeout(confirmation_packet));
}

void DpaRequest::ProcessResponseMessage(const DpaMessage& response_message) {
  status_ = kProcessed;
  delete response_message_;
  response_message_ = new DpaMessage(response_message);
}

void DpaRequest::ProcessReceivedMessage(const DpaMessage& received_message) {
  if (received_message.MessageDirection() != DpaMessage::kResponse
	  && received_message.MessageDirection() != DpaMessage::kConfirmation)
	throw unexpected_packet_type("Response is expected.");

  status_mutex_.lock();

  if (!IsInProgressStatus(status_)) {
	status_mutex_.unlock();
	throw unexpected_packet_type("Request has not been sent, yet.");
  }

  if (received_message.PeripheralType() != sent_message_->PeripheralType()) {
	status_mutex_.unlock();
	throw unexpected_peripheral("Different peripheral type than in sent message.");
  }

  if ((received_message.CommandCode() & ~0x80) != sent_message_->CommandCode()) {
	status_mutex_.unlock();
	throw unexpected_command("Different command code than in sent message.");
  }

  auto message_direction = received_message.MessageDirection();
  if (message_direction == DpaMessage::kConfirmation)
	ProcessConfirmationMessage(received_message);
  else
	ProcessResponseMessage(received_message);

  status_mutex_.unlock();
}

void DpaRequest::CheckTimeout() {
  if (status_ == kProcessed
	  || status_ == kCreated) {
	return;
  }

  if (IsTimeout()) {
	SetStatus(kTimeout);
  }
}

DpaRequest::DpaRequestStatus DpaRequest::Status() {
  CheckTimeout();
  return status_;
}

bool DpaRequest::IsInProgress() {
  return IsInProgressStatus(Status());
}

int32_t DpaRequest::EstimatedTimeout(const DpaMessage& confirmation_packet) {
  int32_t estimated_timeout_ms;
  int32_t response_time_slot_length_ms;

  if (confirmation_packet.MessageDirection() != DpaMessage::kConfirmation)
	throw std::invalid_argument("Parameter is not a confirmation packet.");

  auto iFace = confirmation_packet.DpaPacket().DpaResponsePacket_t.DpaMessage.IFaceConfirmation;

  estimated_timeout_ms = (iFace.Hops + 1) * iFace.TimeSlotLength * 10;
  if (iFace.TimeSlotLength == 20) {
	response_time_slot_length_ms = 200;
  }
  else {
	if (iFace.TimeSlotLength > 6)
	  response_time_slot_length_ms = 100;
	else
	  response_time_slot_length_ms = 50;
  }
  estimated_timeout_ms += (iFace.HopsResponse + 1) * response_time_slot_length_ms + 40;
  return estimated_timeout_ms;
}

void DpaRequest::SetTimeoutForCurrentRequest(int32_t extra_time_in_ms) {
  if (timeout_ms_ == -1) {
	expected_duration_ms_ = timeout_ms_;
	return;        //Infinite time
  }
  start_time_ = std::chrono::system_clock::now();
  expected_duration_ms_ = timeout_ms_ + extra_time_in_ms;
}

bool DpaRequest::IsTimeout() const {
  if (expected_duration_ms_ == -1)
	return false;
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
	  std::chrono::system_clock::now() - start_time_);
  return duration.count() > expected_duration_ms_;
}

void DpaRequest::SetStatus(DpaRequest::DpaRequestStatus status) {
  status_mutex_.lock();
  status_ = status;
  status_mutex_.unlock();
}

bool DpaRequest::IsInProgressStatus(DpaRequestStatus status) {
  switch (status) {
	case kSent:
	case kConfirmation:
	  return true;
	default:
	  return false;
  }
}


