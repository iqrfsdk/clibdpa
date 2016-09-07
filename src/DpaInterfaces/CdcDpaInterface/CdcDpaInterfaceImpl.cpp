#include <stdexcept>
#include <mutex>
#include "CdcDpaInterfaceImpl.h"

CdcDpaInterfaceImpl* CdcDpaInterfaceImpl::instance_;

CdcDpaInterfaceImpl::CdcDpaInterfaceImpl()
	: is_initialized_(false) {

}

CdcDpaInterfaceImpl::~CdcDpaInterfaceImpl() {
  if (is_initialized_) {
	Close();
  }
}

void CdcDpaInterfaceImpl::Open(std::string device) {
  if (is_initialized_)
	throw std::logic_error("Interface is already initialized.");

  InitCdcParser(device);

  is_initialized_ = true;
}

void CdcDpaInterfaceImpl::Close() {
  is_initialized_ = false;
}

void CdcDpaInterfaceImpl::InitCdcParser(std::string device) {
  cdc_parser_ = std::unique_ptr<CDCImpl>(new CDCImpl(device.c_str()));
  auto test_result = cdc_parser_->test();
  if (!test_result) {
	throw std::logic_error("Interface has not been opened.");
  }

  cdc_parser_->registerAsyncMsgListener(CdcListenerWrapper);
  CdcDpaInterfaceImpl::instance_ = this;
}


int32_t CdcDpaInterfaceImpl::SendRequest(unsigned char* data, uint32_t length) {
  if (!is_initialized_)
	return -1;

  auto response = cdc_parser_->sendData(data, length);
  if (response != OK)
	return -1;
  return 0;
}

int CdcDpaInterfaceImpl::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
  std::lock_guard<std::mutex> lock(callbackFunctionMutex_);
  response_callback_ = function;
  return 0;
}

void CdcDpaInterfaceImpl::CdcListenerWrapper(unsigned char* data, uint32_t length) {
  CdcDpaInterfaceImpl::instance_->CdcListener(data, length);
}

void CdcDpaInterfaceImpl::CdcListener(unsigned char* data, uint32_t length) {
  std::lock_guard<std::mutex> lock(callbackFunctionMutex_);        //TODO:asi se da vyresit jinak
  auto callback = response_callback_;
  if (callback) {
	callback(data, length);
  }
}



