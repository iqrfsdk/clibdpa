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

#include "IDpaHandler2.h"
#include "IChannel.h"
#include "DpaTransaction2.h"

class DpaHandler2 : public IDpaHandler2 {
public:
  DpaHandler2( IChannel* iqrfInterface );
  virtual ~DpaHandler2();
  std::shared_ptr<IDpaTransaction2> executeDpaTransaction( const DpaMessage& request, int32_t timeout,
    IDpaTransactionResult2::ErrorCode defaultError) override;
  int getTimeout() const override;
  void setTimeout( int timeout ) override;
  IDpaTransaction2::RfMode getRfCommunicationMode() const override;
  void setRfCommunicationMode( IDpaTransaction2::RfMode rfMode ) override;
  IDpaTransaction2::TimingParams getTimingParams() const override;
  void setTimingParams( IDpaTransaction2::TimingParams params ) override;
  IDpaTransaction2::FrcResponseTime getFrcResponseTime() const override;
  void setFrcResponseTime( IDpaTransaction2::FrcResponseTime frcResponseTime ) override;
  void registerAsyncMessageHandler( const std::string& serviceId, AsyncMessageHandlerFunc fun ) override;
  void unregisterAsyncMessageHandler( const std::string& serviceId ) override;
  int getDpaQueueLen() const override;
  void registerAnyMessageHandler(const std::string& serviceId, AnyMessageHandlerFunc fun) override;
  void unregisterAnyMessageHandler(const std::string& serviceId) override;
private:
  class Imp;
  Imp *m_imp = nullptr;
};
