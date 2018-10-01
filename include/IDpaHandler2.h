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
#include "IDpaTransaction2.h"
#include <functional>

class IDpaHandler2
{
public:

  // Timing constants

  /// Default timeout
  static const int DEFAULT_TIMEOUT = 500;
  /// Minimal timeout used if required by user is too low
  static const int MINIMAL_TIMEOUT = 200;
  /// Zero value used to indicate infinit timeout in special cases (discovery)
  static const int INFINITE_TIMEOUT = 0;
  /// An extra timeout added to timeout from a confirmation packet.
  static const int32_t SAFETY_TIMEOUT_MS = 40;
  /// Asynchronous DPA message handler functional type
  typedef std::function<void( const DpaMessage& dpaMessage )> AsyncMessageHandlerFunc;
  /// 0 > timeout - use default, 0 == timeout - use infinit, 0 < timeout - user value
  virtual std::shared_ptr<IDpaTransaction2> executeDpaTransaction( const DpaMessage& request, int32_t timeout,
    IDpaTransactionResult2::ErrorCode defaultError = IDpaTransactionResult2::TRN_OK) = 0;
  virtual int getTimeout() const = 0;
  virtual void setTimeout( int timeout ) = 0;
  virtual IDpaTransaction2::RfMode getRfCommunicationMode() const = 0;
  virtual void setRfCommunicationMode( IDpaTransaction2::RfMode rfMode ) = 0;
  virtual IDpaTransaction2::TimingParams getTimingParams() const = 0;
  virtual void setTimingParams( IDpaTransaction2::TimingParams params ) = 0;
  virtual IDpaTransaction2::FrcResponseTime getFrcResponseTime() const = 0;
  virtual void setFrcResponseTime( IDpaTransaction2::FrcResponseTime frcResponseTime ) = 0;
  virtual void registerAsyncMessageHandler( const std::string& serviceId, AsyncMessageHandlerFunc fun ) = 0;
  virtual void unregisterAsyncMessageHandler( const std::string& serviceId ) = 0;
  virtual ~IDpaHandler2() {}
};
