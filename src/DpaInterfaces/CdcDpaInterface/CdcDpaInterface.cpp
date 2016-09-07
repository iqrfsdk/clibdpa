#include "include/CdcDpaInterface.h"
#include "CdcDpaInterfaceImpl.h"


CdcDpaInterface::CdcDpaInterface() {
  cdcDpaInterfaceImpl_ = std::unique_ptr<CdcDpaInterfaceImpl>(new CdcDpaInterfaceImpl());
}

CdcDpaInterface::~CdcDpaInterface() {

}

void CdcDpaInterface::Open(std::string device) {
  cdcDpaInterfaceImpl_->Open(device);
}

void CdcDpaInterface::Close() {
  cdcDpaInterfaceImpl_->Close();
}

int32_t CdcDpaInterface::SendRequest(unsigned char* data, uint32_t length) {
  return cdcDpaInterfaceImpl_->SendRequest(data, length);
}

int CdcDpaInterface::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
  return cdcDpaInterfaceImpl_->RegisterResponseHandler(function);
}


