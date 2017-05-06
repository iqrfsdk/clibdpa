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

#include "DpaTransaction.h"
#include "unexpected_peripheral.h"
#include "DpaRequest.h"
#include "unexpected_packet_type.h"
#include "unexpected_command.h"

DpaRequest::DpaRequest()
	: status_(kCreated), sent_message_(nullptr), response_message_(nullptr),
	  expected_duration_ms_(0), timeout_ms_(-1), current_communication_mode_(kStd), dpaTransaction_(nullptr)
{
}

DpaRequest::DpaRequest(DpaTransaction* dpaTransaction)
	: status_(kCreated), sent_message_(nullptr), response_message_(nullptr),
	  expected_duration_ms_(0), timeout_ms_(-1), current_communication_mode_(kStd), dpaTransaction_(dpaTransaction)
{
}

DpaRequest::~DpaRequest()
{
	delete sent_message_;
	delete response_message_;
}

void DpaRequest::ProcessSentMessage(const DpaMessage& sent_message)
{
	if (status_ != kCreated)
	{
		throw std::logic_error("Sent message already set.");
	}

	SetStatus(kSent);
	delete sent_message_;
	sent_message_ = new DpaMessage(sent_message);
	SetTimeoutForCurrentRequest();
}

void DpaRequest::ProcessConfirmationMessage(const DpaMessage& confirmation_packet)
{
	if (confirmation_packet.NodeAddress() == DpaMessage::kBroadCastAddress)
	{
		status_ = kConfirmationBroadcast;
	}
	else
	{
		status_ = kConfirmation;
	}
	SetTimeoutForCurrentRequest(EstimatedTimeout(confirmation_packet));
}

void DpaRequest::ProcessResponseMessage(const DpaMessage& response_message)
{
	status_ = kProcessed;
	delete response_message_;
	response_message_ = new DpaMessage(response_message);
}

void DpaRequest::ProcessReceivedMessage(const DpaMessage& received_message)
{
	if (received_message.MessageDirection() != DpaMessage::kResponse
		&& received_message.MessageDirection() != DpaMessage::kConfirmation)
		throw unexpected_packet_type("Response is expected.");

	std::lock_guard<std::mutex> lck(status_mutex_);

	if (!IsInProgressStatus(status_))
	{
		throw unexpected_packet_type("Request has not been sent, yet.");
	}

	if (received_message.PeripheralType() != sent_message_->PeripheralType())
	{
		throw unexpected_peripheral("Different peripheral type than in sent message.");
	}

	if ((received_message.CommandCode() & ~0x80) != sent_message_->CommandCode())
	{
		throw unexpected_command("Different command code than in sent message.");
	}

	auto message_direction = received_message.MessageDirection();
	if (message_direction == DpaMessage::kConfirmation)
	{
		if (dpaTransaction_)
		{
			dpaTransaction_->processConfirmationMessage(received_message);
		}
		ProcessConfirmationMessage(received_message);
	}
	else
	{
		if (dpaTransaction_)
		{
			dpaTransaction_->processResponseMessage(received_message);
		}
		ProcessResponseMessage(received_message);
	}
}

int32_t DpaRequest::CheckTimeout()
{
	int32_t remains(0);

	if (status_ == kProcessed
		|| status_ == kConfirmationBroadcast
		|| status_ == kCreated)
	{
		if (status_ == kConfirmationBroadcast)
			SetStatus(kProcessed);
		return remains;
	}

	bool timeout(false);

	if (expected_duration_ms_ != -1)
	{
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - start_time_);
		remains = expected_duration_ms_ - duration.count();
		timeout = remains < 0;
	}

	if (timeout)
	{
		if (status_ == kConfirmationBroadcast)
			SetStatus(kProcessed);
		else
			SetStatus(kTimeout);
	}

	return remains;
}

DpaRequest::DpaRequestStatus DpaRequest::Status()
{
	std::lock_guard<std::mutex> lck(status_mutex_);
	CheckTimeout();
	return status_;
}

bool DpaRequest::IsInProgress()
{
	return IsInProgressStatus(Status());
}

bool DpaRequest::IsInProgress(int32_t& expected_duration)
{
	std::lock_guard<std::mutex> lck(status_mutex_);
	expected_duration = CheckTimeout();
	return IsInProgressStatus(status_);
}


DpaRequest::IqrfRfCommunicationMode DpaRequest::IqrfRfMode() const
{
	return current_communication_mode_;
}

void DpaRequest::IqrfRfMode(IqrfRfCommunicationMode mode)
{
	current_communication_mode_ = mode;
}

int32_t DpaRequest::EstimatedTimeout(const DpaMessage& confirmation_packet)
{
	if (confirmation_packet.MessageDirection() != DpaMessage::kConfirmation)
		throw std::invalid_argument("Parameter is not a confirmation packet.");

	auto iFace = confirmation_packet.DpaPacket().DpaResponsePacket_t.DpaMessage.IFaceConfirmation;

	if (current_communication_mode_ == kLp)
	{
		return EstimateLpTimeout(iFace.Hops, iFace.HopsResponse, iFace.TimeSlotLength);
	}
	return EstimateStdTimeout(iFace.Hops, iFace.HopsResponse, iFace.TimeSlotLength);
}

int32_t DpaRequest::EstimateStdTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot, int32_t response = -1)
{
	auto estimated_timeout_ms = (hops + 1) * timeslot * 10;
	int32_t response_time_slot_length_ms;
	if (timeslot == 20)
	{
		response_time_slot_length_ms = 200;
	}
	else
	{
		response_time_slot_length_ms = 50;
	}
	estimated_timeout_ms += (hops_response + 1) * response_time_slot_length_ms + 40;
	return estimated_timeout_ms;
}

int32_t DpaRequest::EstimateLpTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot, int32_t response = -1)
{
	auto estimated_timeout_ms = (hops + 1) * timeslot * 10;
	int32_t response_time_slot_length_ms;
	if (timeslot == 20)
	{
		response_time_slot_length_ms = 200;
	}
	else
	{
		response_time_slot_length_ms = 100;
	}
	estimated_timeout_ms += (hops_response + 1) * response_time_slot_length_ms + 40;
	return estimated_timeout_ms;
}

void DpaRequest::SetTimeoutForCurrentRequest(int32_t extra_time_in_ms)
{
	if (status_ != kConfirmationBroadcast && timeout_ms_ == -1)
	{
		expected_duration_ms_ = timeout_ms_;
		return; //Infinite time
	}
	start_time_ = std::chrono::system_clock::now();
	expected_duration_ms_ = timeout_ms_ + extra_time_in_ms;
}

//bool DpaRequest::IsTimeout() const {
//  if (expected_duration_ms_ == -1)
//    return false;
//  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
//    std::chrono::system_clock::now() - start_time_);
//  return duration.count() > expected_duration_ms_;
//}

void DpaRequest::SetStatus(DpaRequest::DpaRequestStatus status)
{
	status_ = status;
}

bool DpaRequest::IsInProgressStatus(DpaRequestStatus status)
{
	switch (status)
	{
	case kSent:
	case kConfirmation: return true;
	default: return false;
	}
}
