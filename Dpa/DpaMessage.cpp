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

#include "DpaMessage.h"
#include "unexpected_packet_type.h"

DpaMessage::DpaMessage(): m_length(0) {
	m_dpa_packet = new DpaPacket_t();
}

DpaMessage::DpaMessage(const unsigned char* data, uint8_t length) {
	m_dpa_packet = new DpaPacket_t();
	DataToBuffer(data, length);
}

DpaMessage::DpaMessage(const std::basic_string<unsigned char>& message) {
	m_dpa_packet = new DpaPacket_t();
	DataToBuffer(message.data(), message.length());
}

DpaMessage::DpaMessage(const DpaMessage& other): m_length(other.m_length) {
	m_dpa_packet = new DpaPacket_t();
	std::copy(other.m_dpa_packet->Buffer, other.m_dpa_packet->Buffer + other.m_length, this->m_dpa_packet->Buffer);
}

DpaMessage::~DpaMessage() {
	delete m_dpa_packet;
}

DpaMessage& DpaMessage::operator=(const DpaMessage& other) {
	if (this == &other)
		return *this;

	delete m_dpa_packet;
	m_dpa_packet = new DpaPacket_t();

	std::copy(other.m_dpa_packet->Buffer, other.m_dpa_packet->Buffer + other.m_length, this->m_dpa_packet->Buffer);
	m_length = other.m_length;
  
	return *this;
}

DpaMessage& DpaMessage::operator=(const std::basic_string<unsigned char>& message) {
	DataToBuffer(message.data(), message.length());
	return *this;
}

DpaMessage::MessageType DpaMessage::MessageDirection() const {
	if (m_length < kCommandIndex)
		return kRequest;

	if (PeripheralCommand() & 0x80)
		return kResponse;

	if (m_length > kStatusCodeIndex && IsConfirmationMessage())
		return kConfirmation;

	return kRequest;
}

void DpaMessage::FillFromResponse(const unsigned char* data, uint8_t length) {
	if (length == 0)
		throw std::invalid_argument("Invalid length.");

	DataToBuffer(data, length);
}

void DpaMessage::DataToBuffer(const unsigned char* data, uint8_t length) {
	if (length == 0)
		return;

	if (data == nullptr)
		throw std::invalid_argument("Data argument can not be null.");

	if (length > kMaxDpaMessageSize)
		throw std::length_error("Not enough space for this data.");

	std::copy(data, data + length, m_dpa_packet->Buffer);
	m_length = length;
}

void DpaMessage::SetLength(int length) {
	if (length > kMaxDpaMessageSize || length <= 0)
		throw std::length_error("Invalid length value.");
	m_length = length;
}

TErrorCodes DpaMessage::ResponseCode() const {
	if (MessageDirection() != kResponse)
		throw unexpected_packet_type("Only response packet has response error defined.");

	return TErrorCodes(m_dpa_packet->DpaResponsePacket_t.ResponseCode);
}

bool DpaMessage::IsConfirmationMessage() const {
	auto responseCode = TErrorCodes(m_dpa_packet->DpaResponsePacket_t.ResponseCode);

	if (responseCode == STATUS_CONFIRMATION)
		return true;

	return false;
}
