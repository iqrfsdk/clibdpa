#include <iostream>
#include "CdcDpaInterface.h"

int32_t CdcDpaInterface::SendRequest(unsigned char* data, uint32_t length) {
	//TODO: doplnit test na is initialized

	auto response = cdc_impl_->sendData(data, length);
	if (response != OK)
		return -1;
	return 0;
}

int CdcDpaInterface::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
	response_callback_ = function;
}

CdcDpaInterface::CdcDpaInterface()
		: cdc_impl_(nullptr), is_initialized_(false) {
}

CdcDpaInterface::~CdcDpaInterface() {
	delete cdc_impl_;
}

void CdcDpaInterface::Init(CDCImpl* cdc_impl) {
	if (is_initialized_)
		throw std::logic_error("Interface is already initialized.");

	if (cdc_impl == nullptr)
		throw std::invalid_argument("CDC Impl argument can not be NULL.");

	cdc_impl_ = cdc_impl;
	is_initialized_ = true;
}

void CdcDpaInterface::ReceiveData(unsigned char* data, uint32_t length) {
	if (response_callback_)
		response_callback_(data, length);
}



