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
#include "IChannel.h"
#include "DpaTransaction2.h"

class IDpaHandler2
{
public:
  /// Asynchronous DPA message handler functional type
  typedef std::function<void( const DpaMessage& dpaMessage )> AsyncMessageHandlerFunc;
  /// 0 > timeout - use default, 0 == timeout - use infinit, 0 < timeout - user value
  virtual std::shared_ptr<IDpaTransaction2> executeDpaTransaction( const DpaMessage& request, int32_t timeout ) = 0;
  virtual int getTimeout() const = 0;
  virtual void setTimeout( int timeout ) = 0;
  virtual IDpaTransaction2::RfMode getRfCommunicationMode() const = 0;
  virtual void setRfCommunicationMode( IDpaTransaction2::RfMode rfMode ) = 0;
  virtual IDpaTransaction2::FRC_TimingParams getFrcTiming() const = 0;
  virtual void setFrcTiming( IDpaTransaction2::FRC_TimingParams params ) = 0;
  virtual void registerAsyncMessageHandler( const std::string& serviceId, AsyncMessageHandlerFunc fun ) = 0;
  virtual void unregisterAsyncMessageHandler( const std::string& serviceId ) = 0;
  virtual ~IDpaHandler2() {}
};

class DpaHandler2 : public IDpaHandler2 {
public:
  DpaHandler2( IChannel* iqrfInterface );
  virtual ~DpaHandler2();
  std::shared_ptr<IDpaTransaction2> executeDpaTransaction( const DpaMessage& request, int32_t timeout ) override;
  int getTimeout() const override;
  void setTimeout( int timeout ) override;
  IDpaTransaction2::RfMode getRfCommunicationMode() const override;
  void setRfCommunicationMode( IDpaTransaction2::RfMode rfMode ) override;
  IDpaTransaction2::FRC_TimingParams getFrcTiming() const override;
  void setFrcTiming( IDpaTransaction2::FRC_TimingParams params ) override;
  void registerAsyncMessageHandler( const std::string& serviceId, AsyncMessageHandlerFunc fun ) override;
  void unregisterAsyncMessageHandler( const std::string& serviceId ) override;
private:
  class Imp;
  Imp *m_imp = nullptr;
};
