/**
* Copyright 2015-2018 MICRORISC s.r.o.
* Copyright 2018 IQRF Tech s.r.o.
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

#pragma once

#include "DpaMessage.h"
#include <chrono>
#include <sstream>

class IDpaTransactionResult2
{
public:
  enum ErrorCode {
    // transaction handling
    TRN_ERROR_IFACE_EXCLUSIVE_ACCESS = -8,
    TRN_ERROR_BAD_RESPONSE = -7,
    TRN_ERROR_BAD_REQUEST = -6,
    TRN_ERROR_IFACE_BUSY = -5,
    TRN_ERROR_IFACE = -4,
    TRN_ERROR_ABORTED = -3,
    TRN_ERROR_IFACE_QUEUE_FULL = -2,
    TRN_ERROR_TIMEOUT = -1,
    TRN_OK = STATUS_NO_ERROR,
    // DPA response erors
    TRN_ERROR_FAIL = ERROR_FAIL,
    TRN_ERROR_PCMD = ERROR_PCMD,
    TRN_ERROR_PNUM = ERROR_PNUM,
    TRN_ERROR_ADDR = ERROR_ADDR,
    TRN_ERROR_DATA_LEN = ERROR_DATA_LEN,
    TRN_ERROR_DATA = ERROR_DATA,
    TRN_ERROR_HWPID = ERROR_HWPID,
    TRN_ERROR_NADR = ERROR_NADR,
    TRN_ERROR_IFACE_CUSTOM_HANDLER = ERROR_IFACE_CUSTOM_HANDLER,
    TRN_ERROR_MISSING_CUSTOM_DPA_HANDLER = ERROR_MISSING_CUSTOM_DPA_HANDLER,
    TRN_ERROR_USER_TO = ERROR_USER_TO,
    TRN_STATUS_CONFIRMATION = STATUS_CONFIRMATION,
    TRN_ERROR_USER_FROM = ERROR_USER_FROM
  };

  virtual int getErrorCode() const = 0;
  virtual void overrideErrorCode( ErrorCode err ) = 0;
  virtual std::string getErrorString() const = 0;
  virtual const DpaMessage& getRequest() const = 0;
  virtual const DpaMessage& getConfirmation() const = 0;
  virtual const DpaMessage& getResponse() const = 0;
  virtual const std::chrono::time_point<std::chrono::system_clock>& getRequestTs() const = 0;
  virtual const std::chrono::time_point<std::chrono::system_clock>& getConfirmationTs() const = 0;
  virtual const std::chrono::time_point<std::chrono::system_clock>& getResponseTs() const = 0;
  virtual bool isConfirmed() const = 0;
  virtual bool isResponded() const = 0;
  virtual ~IDpaTransactionResult2() {};

  static std::string errorCode(int errorCode)
  {
    switch (errorCode) {

    case TRN_ERROR_IFACE_EXCLUSIVE_ACCESS:
      return "ERROR_IFACE_EXCLUSIVE_ACCESS";
    case TRN_ERROR_BAD_RESPONSE:
      return "BAD_RESPONSE";
    case TRN_ERROR_BAD_REQUEST:
      return "BAD_REQUEST";
    case TRN_ERROR_IFACE_BUSY:
      return "ERROR_IFACE_BUSY";
    case TRN_ERROR_IFACE:
      return "ERROR_IFACE";
    case TRN_ERROR_ABORTED:
      return "ERROR_ABORTED";
    case TRN_ERROR_IFACE_QUEUE_FULL:
      return "ERROR_IFACE_QUEUE_FULL";
    case TRN_ERROR_TIMEOUT:
      return "ERROR_TIMEOUT";
    case TRN_OK:
      return "ok";
    case TRN_ERROR_FAIL:
      return "ERROR_FAIL";
    case TRN_ERROR_PCMD:
      return "ERROR_PCMD";
    case TRN_ERROR_PNUM:
      return "ERROR_PNUM";
    case TRN_ERROR_ADDR:
      return "ERROR_ADDR";
    case TRN_ERROR_DATA_LEN:
      return "ERROR_DATA_LEN";
    case TRN_ERROR_DATA:
      return "ERROR_DATA";
    case TRN_ERROR_HWPID:
      return "ERROR_HWPID";
    case TRN_ERROR_NADR:
      return "ERROR_NADR";
    case TRN_ERROR_IFACE_CUSTOM_HANDLER:
      return "ERROR_IFACE_CUSTOM_HANDLER";
    case TRN_ERROR_MISSING_CUSTOM_DPA_HANDLER:
      return "ERROR_MISSING_CUSTOM_DPA_HANDLER";
    case TRN_ERROR_USER_TO:
      return "ERROR_USER_TO";
    case TRN_STATUS_CONFIRMATION:
      return "STATUS_CONFIRMATION";
    case TRN_ERROR_USER_FROM:
    default:
      std::ostringstream os;
      os << "TRN_ERROR_USER_" << std::hex << errorCode;
      return os.str();
    }
  }

};
