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
//#include <sstream>

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

//std::string DpaTransactionResult2::getErrorString() const
//{
//  switch ( m_errorCode ) {
//
//    case TRN_ERROR_BAD_RESPONSE:
//      return "BAD_RESPONSE";
//    case TRN_ERROR_BAD_REQUEST:
//      return "BAD_REQUEST";
//    case TRN_ERROR_IFACE_BUSY:
//      return "ERROR_IFACE_BUSY";
//    case TRN_ERROR_IFACE:
//      return "ERROR_IFACE";
//    case TRN_ERROR_ABORTED:
//      return "ERROR_ABORTED";
//    case TRN_ERROR_IFACE_QUEUE_FULL:
//      return "ERROR_IFACE_QUEUE_FULL";
//    case TRN_ERROR_TIMEOUT:
//      return "ERROR_TIMEOUT";
//    case TRN_OK:
//      return "ok";
//    case TRN_ERROR_FAIL:
//      return "ERROR_FAIL";
//    case TRN_ERROR_PCMD:
//      return "ERROR_PCMD";
//    case TRN_ERROR_PNUM:
//      return "ERROR_PNUM";
//    case TRN_ERROR_ADDR:
//      return "ERROR_ADDR";
//    case TRN_ERROR_DATA_LEN:
//      return "ERROR_DATA_LEN";
//    case TRN_ERROR_DATA:
//      return "ERROR_DATA";
//    case TRN_ERROR_HWPID:
//      return "ERROR_HWPID";
//    case TRN_ERROR_NADR:
//      return "ERROR_NADR";
//    case TRN_ERROR_IFACE_CUSTOM_HANDLER:
//      return "ERROR_IFACE_CUSTOM_HANDLER";
//    case TRN_ERROR_MISSING_CUSTOM_DPA_HANDLER:
//      return "ERROR_MISSING_CUSTOM_DPA_HANDLER";
//    case TRN_ERROR_USER_TO:
//      return "ERROR_USER_TO";
//    case TRN_STATUS_CONFIRMATION:
//      return "STATUS_CONFIRMATION";
//    case TRN_ERROR_USER_FROM:
//    default:
//      std::ostringstream os;
//      os << "TRN_ERROR_USER_" << std::hex << m_errorCode;
//      return os.str();
//  }
//}

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