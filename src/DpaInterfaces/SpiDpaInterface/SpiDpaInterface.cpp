#include "include/SpiDpaInterface.h"
#include "SpiDpaInterfaceImpl.h"

SpiDpaInterface::~SpiDpaInterface() {
  delete spiDpaInterfaceImpl_;
}

int32_t SpiDpaInterface::SendRequest(unsigned char* data, uint32_t length) {
  return spiDpaInterfaceImpl_->SendRequest(data, length);
}

int32_t SpiDpaInterface::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
  return spiDpaInterfaceImpl_->RegisterResponseHandler(function);
}

SpiDpaInterface::SpiDpaInterface() {
  spiDpaInterfaceImpl_ = new SpiDpaInterfaceImpl();
}

void SpiDpaInterface::Open(std::string device) {
  spiDpaInterfaceImpl_->Open(device);
}

void SpiDpaInterface::Close() {
  spiDpaInterfaceImpl_->Close();
}





