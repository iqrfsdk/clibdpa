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

#include "DpaTransactionResult.h"
#include "DPA.h"

#include <sstream>


DpaTransactionResult::DpaTransactionResult(int errorCode) {
  m_errorCode = errorCode;
}

DpaTransactionResult::~DpaTransactionResult() {}


int DpaTransactionResult::getErrorCode() const {
  return m_errorCode;
}

std::string DpaTransactionResult::getErrorString() const {
  switch (m_errorCode) {
  case -4:
	return "ERROR_IFACE";
  case -3:
	return "ERROR_ABORTED";
  case -2:
	return "ERROR_PROMISE_TIMEOUT";
  case -1:
	return "ERROR_TIMEOUT";
  case STATUS_NO_ERROR:
	return "STATUS_NO_ERROR";
  case ERROR_FAIL:
	return "ERROR_FAIL";
  case ERROR_PCMD:
	return "ERROR_PCMD";
  case ERROR_PNUM:
	return "ERROR_PNUM";
  case ERROR_ADDR:
	return "ERROR_ADDR";
  case ERROR_DATA_LEN:
	return "ERROR_DATA_LEN";
  case ERROR_DATA:
	return "ERROR_DATA";
  case ERROR_HWPID:
	return "ERROR_HWPID";
  case ERROR_NADR:
	return "ERROR_NADR";
  case ERROR_IFACE_CUSTOM_HANDLER:
	return "ERROR_IFACE_CUSTOM_HANDLER";
  case ERROR_MISSING_CUSTOM_DPA_HANDLER:
	return "ERROR_MISSING_CUSTOM_DPA_HANDLER";
  case ERROR_USER_TO:
	return "ERROR_USER_TO";
  case STATUS_CONFIRMATION:
	return "STATUS_CONFIRMATION";
  case ERROR_USER_FROM:
  default:
	std::ostringstream os;
	os << std::hex << m_errorCode;
	return os.str();
  }
}