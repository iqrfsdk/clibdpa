#include "DpaMessage.h"
#include "unexpected_packet_type.h"


DpaMessage::DpaMessage()
  : length_(0) {
  dpa_packet_ = new DpaPacket_t();
}

DpaMessage::DpaMessage(const DpaMessage& other)
  : length_(other.length_) {
  dpa_packet_ = new DpaPacket_t();
  std::copy(other.dpa_packet_->Buffer, other.dpa_packet_->Buffer + other.length_, this->dpa_packet_->Buffer);
}

DpaMessage::~DpaMessage() {
  delete dpa_packet_;
}

DpaMessage& DpaMessage::operator=(const DpaMessage& other) {
  if (this == &other)
    return *this;
  delete dpa_packet_;
  dpa_packet_ = new DpaPacket_t();
  std::copy(other.dpa_packet_->Buffer, other.dpa_packet_->Buffer + length_, this->dpa_packet_->Buffer);
  length_ = other.length_;
  return *this;
}

DpaMessage::MessageType DpaMessage::MessageDirection() const {
  if (length_ < kCommandIndex)
    return kRequest;

  if (CommandCode() & 0x80)
    return kResponse;

  if (length_ > kStatusCodeIndex && IsConfirmationMessage())
    return kConfirmation;

  return kRequest;
}

void DpaMessage::FillFromResponse(unsigned char* data, uint32_t length) {
  if (length == 0)
    throw std::invalid_argument("Invalid length.");

  AddDataToBuffer(data, length);
}

void DpaMessage::AddDataToBuffer(const unsigned char* data, uint32_t length) {
  if (length == 0)
    return;

  if (data == nullptr)
    throw std::invalid_argument("Data argument can not be null.");

  if (length_ + length > kMaxDpaMessageSize)
    throw std::length_error("Not enough space for this data.");

  std::copy(data, data + length, dpa_packet_->Buffer);
  length_ += length;
}

TErrorCodes DpaMessage::ResponseCode() const {
  if (MessageDirection() != kResponse)
    throw unexpected_packet_type("Only response packet has response error defined.");

  return TErrorCodes(dpa_packet_->DpaResponsePacket_t.ResponseCode);
}

bool DpaMessage::IsConfirmationMessage() const {
  auto responseCode = TErrorCodes(dpa_packet_->DpaResponsePacket_t.ResponseCode);

  if (responseCode == STATUS_CONFIRMATION)
    return true;
  return false;
}
