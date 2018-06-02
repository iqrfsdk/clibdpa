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

#include "IDpaTransactionResult2.h"

class DpaTransactionResult2 : public IDpaTransactionResult2
{
private:
  /// original request creating the transaction
  DpaMessage m_request;
  /// received  confirmation
  DpaMessage m_confirmation;
  /// received  response
  DpaMessage m_response;
  /// request timestamp
  std::chrono::time_point<std::chrono::system_clock> m_request_ts;
  /// confirmation timestamp
  std::chrono::time_point<std::chrono::system_clock> m_confirmation_ts;
  /// response timestamp
  std::chrono::time_point<std::chrono::system_clock> m_response_ts;
  /// overall error code
  int m_errorCode = ErrorCode::TRN_ERROR_ABORTED;
  /// response code
  int m_responseCode = ErrorCode::TRN_OK;
  /// received and set response flag
  bool m_isResponded = false;
  /// received and set confirmation flag
  int m_isConfirmed = false;

public:
  DpaTransactionResult2() = delete;
  DpaTransactionResult2( const DpaMessage& request );
  int getErrorCode() const override;
  void overrideErrorCode( ErrorCode err ) override;
  std::string getErrorString() const override;
  const DpaMessage& getRequest() const override;
  const DpaMessage& getConfirmation() const override;
  const DpaMessage& getResponse() const override;
  const std::chrono::time_point<std::chrono::system_clock>& getRequestTs() const override;
  const std::chrono::time_point<std::chrono::system_clock>& getConfirmationTs() const override;
  const std::chrono::time_point<std::chrono::system_clock>& getResponseTs() const override;
  bool isConfirmed() const override;
  bool isResponded() const override;
  void setConfirmation( const DpaMessage& confirmation );
  void setResponse( const DpaMessage& response );
  void setErrorCode( int errorCode );
};
