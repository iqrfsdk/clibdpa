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

#pragma once

#include "DpaMessage.h"
#include <memory>
#include <functional>
#include <chrono>

class IDpaTransactionResult2
{
public:
  enum ErrorCode {
    // transaction handling
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
};

class IDpaTransaction2
{
public:
  virtual ~IDpaTransaction2() {}
  /// wait for result
  virtual std::unique_ptr<IDpaTransactionResult2> get() = 0;
  /// abort the transaction immediately 
  virtual void abort() = 0;
};

class IDpaHandler2
{
public:
  enum RfMode {
    kStd,
    kLp
  };

  /// Default timeout
  static const int DEFAULT_TIMEOUT = 500;
  /// Minimal timeout used if required by user is too low
  static const int MINIMAL_TIMEOUT = 200;
  /// Zero value used to indicate infinit timeout in special cases (discovery)
  static const int INFINITE_TIMEOUT = 0;
  /// An extra timeout added to timeout from a confirmation packet.
  static const int32_t SAFETY_TIMEOUT_MS = 40;

  /// Asynchronous DPA message handler functional type
  typedef std::function<void(const DpaMessage& dpaMessage)> AsyncMessageHandlerFunc;

  /// 0 > timeout - use default, 0 == timeout - use infinit, 0 < timeout - user value
  virtual std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request, int32_t timeout) = 0;
  virtual int getTimeout() const = 0;
  virtual void setTimeout(int timeout) = 0;
  virtual RfMode getRfCommunicationMode() const = 0;
  virtual void setRfCommunicationMode(RfMode rfMode) = 0;
  virtual void registerAsyncMessageHandler(const std::string& serviceId, AsyncMessageHandlerFunc fun) = 0;
  virtual void unregisterAsyncMessageHandler(const std::string& serviceId) = 0;

  virtual ~IDpaHandler2() {}
};

class IChannel;

class DpaHandler2 : public IDpaHandler2 {
public:
  DpaHandler2(IChannel* iqrfInterface);
  ~DpaHandler2();
  std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request, int32_t timeout) override;
  int getTimeout() const override;
  void setTimeout(int timeout) override;
  RfMode getRfCommunicationMode() const override;
  void setRfCommunicationMode(RfMode rfMode) override;
  void registerAsyncMessageHandler(const std::string& serviceId, AsyncMessageHandlerFunc fun) override;
  void unregisterAsyncMessageHandler(const std::string& serviceId) override;
private:
  class Imp;
  Imp *m_imp = nullptr;
};
