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

#include "DpaTransactionResult2.h"
#include "DpaMessage.h"

using namespace std;

/////////////////////////////////////
// class DpaTransactionResult2
/////////////////////////////////////
DpaTransactionResult2::DpaTransactionResult2( const DpaMessage& request )
{
  m_request_ts = std::chrono::system_clock::now();
  m_request = request;
}

int DpaTransactionResult2::getErrorCode() const
{
  if ( m_errorCode != TRN_OK ) {
    return m_errorCode;
  }
  else {
    return m_responseCode;
  }
}

void DpaTransactionResult2::overrideErrorCode( IDpaTransactionResult2::ErrorCode err )
{
  m_errorCode = err;
}

std::string DpaTransactionResult2::getErrorString() const
{
  return errorCode(m_errorCode);
}

const DpaMessage& DpaTransactionResult2::getRequest() const
{
  return m_request;
}

const DpaMessage& DpaTransactionResult2::getConfirmation() const
{
  return m_confirmation;
}

const DpaMessage& DpaTransactionResult2::getResponse() const
{
  return m_response;
}

const std::chrono::time_point<std::chrono::system_clock>& DpaTransactionResult2::getRequestTs() const
{
  return m_request_ts;
}

const std::chrono::time_point<std::chrono::system_clock>& DpaTransactionResult2::getConfirmationTs() const
{
  return m_confirmation_ts;
}

const std::chrono::time_point<std::chrono::system_clock>& DpaTransactionResult2::getResponseTs() const
{
  return m_response_ts;
}

bool DpaTransactionResult2::isConfirmed() const
{
  return m_isConfirmed;
}

bool DpaTransactionResult2::isResponded() const
{
  return m_isResponded;
}

void DpaTransactionResult2::setConfirmation( const DpaMessage& confirmation )
{
  m_confirmation_ts = std::chrono::system_clock::now();
  m_confirmation = confirmation;
  m_isConfirmed = true;
}

void DpaTransactionResult2::setResponse( const DpaMessage& response )
{
  m_response_ts = std::chrono::system_clock::now();
  m_response = response;
  if ( 0 < response.GetLength() ) {
    m_responseCode = response.DpaPacket().DpaResponsePacket_t.ResponseCode;
    m_isResponded = true;
  }
  else {
    m_isResponded = false;
  }
}

void DpaTransactionResult2::setErrorCode( int errorCode )
{
  if ( errorCode == TRN_OK && m_responseCode != TRN_OK ) {
    errorCode = m_responseCode;
  }
  m_errorCode = errorCode;
}