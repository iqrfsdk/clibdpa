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

#include <memory>
#include <queue>
#include <functional>
#include <cstdint>
#include <mutex>
#include <condition_variable>

#include "DpaTask.h"
#include "DpaTransfer.h"
#include "DpaTransaction.h"
#include "DpaMessage.h"
#include "IChannel.h"
#include "DpaTransactionResult.h"

class IDpaTransactionResult2
{
public:
  virtual ~IDpaTransactionResult2() {};
  virtual int getTransactionResult() const = 0;
  virtual int getDpaResult() const = 0;
  virtual const std::string& getResultString() const = 0;
  virtual const DpaMessage& getRequest() const = 0;
  virtual const DpaMessage& getConfirmation() const = 0;
  virtual const DpaMessage& getResponse() const = 0;
  virtual const std::chrono::time_point<std::chrono::system_clock>& getRequestTs() const = 0;
  virtual const std::chrono::time_point<std::chrono::system_clock>& getConfirmationTs() const = 0;
  virtual const std::chrono::time_point<std::chrono::system_clock>& getResponseTs() const = 0;
};

class IDpaTransaction2
{
public:
  virtual ~IDpaTransaction2() {}
  virtual std::unique_ptr<IDpaTransactionResult2> get(uint32_t timeout) = 0;
};

class IDpaHandler2
{
public:
  enum RfMode {
    Std,
    Lp
  };

  /// Asynchronous DPA message handler functional type
  typedef std::function<void(const DpaMessage& dpaMessage)> AsyncMessageHandlerFunc;

  virtual std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request) = 0;
  virtual void killDpaTransaction() = 0;
  virtual int getTimeout() const = 0;
  virtual void setTimeout(int timeout) = 0;
  virtual RfMode getRfCommunicationMode() const = 0;
  virtual void setRfCommunicationMode(RfMode rfMode) = 0;
  virtual void registerAsyncMessageHandler(const std::string& serviceId, AsyncMessageHandlerFunc fun) = 0;
  virtual void unregisterAsyncMessageHandler(const std::string& serviceId) = 0;

  virtual ~IDpaHandler2() {}
};

class DpaHandler2 : public IDpaHandler2 {
public:
  //Timing constants
  enum Timing {
    DEFAULT_TIMING = 200,
    MINIMAL_TIMING = 200,
    INFINITE_TIMING = 0
  };
  DpaHandler2(IChannel* iqrfInterface);
  ~DpaHandler2();
  std::shared_ptr<IDpaTransaction2> executeDpaTransaction(const DpaMessage& request) override;
  void killDpaTransaction() override;
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
