#include "CdcDpaInterface.h"

CdcDpaInterface::CdcDpaInterface(const std::string& portIqrf)
  : m_cdc(portIqrf.c_str())
{
  if (!m_cdc.test()) {
    CDCImplException ex("CDC Test failed");
    throw ex;
  }
}

CdcDpaInterface::~CdcDpaInterface() {

}

int32_t CdcDpaInterface::SendRequest(unsigned char* data, uint32_t length) {
  DSResponse dsResponse = m_cdc.sendData(data, length);
  if (dsResponse != DSResponse::OK)
    return -1;
  return 0;
}

void CdcDpaInterface::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
  responseHandlerFunc_ = function;
  m_cdc.registerAsyncMsgListener([&](unsigned char* data, unsigned int length) {
    responseHandlerFunc_(data, length); });
}


