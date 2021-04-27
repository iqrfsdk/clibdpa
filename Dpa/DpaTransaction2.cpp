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

#include "TaskQueue.h"
#include "DpaHandler2.h"
#include "DpaTransaction2.h"
#include "DpaTransactionResult2.h"
#include "DpaMessage.h"
#include "IqrfTrace.h"
#include "IChannel.h"
#include <iostream>
#include <future>
#include <exception>
#include <utility>

using namespace std;

/////////////////////////////////////
// class DpaTransaction2
/////////////////////////////////////

//This is key timing class. Its methods execute(), get(), processReceivedMessage() runs concurently and has to be synchronized
//
// user calls:
// std::shared_ptr<IDpaTransaction2> dt = m_dpa->executeDpaTransaction(dpaTask->getRequest());
// std::unique_ptr<IDpaTransactionResult2> dtr = dt->get();
//
// When user invokes m_dpa->executeDpaTransaction() this class is constructed and stored by m_dpa to the queue of transactions.
// If there is no pending transaction the first one from the queue is execute() by DpaHandler queue worker thread.
// If the function is not invoked in time required by user, the transaction timer expires and 
// this situation is reported via transaction result error code (coordinator busy or queue full)
//
// execute() gets to point waiting for confirmation or response from coordinator. These messages are handled by processReceivedMessage().
// The function computes expected time set appropriate transaction state and pass controll back to execute() to continue or finish the transaction.
// When transaction is finish the function execute() pass control to get() and it returns transaction result to user.
//
DpaTransaction2::DpaTransaction2( const DpaMessage& request,
  RfMode mode, TimingParams params, int32_t defaultTimeout, int32_t userTimeout, SendDpaMessageFunc sender,
  IDpaTransactionResult2::ErrorCode defaultError)
  : m_dpaTransactionResultPtr( ant_new DpaTransactionResult2( request ) )
  , m_state(DpaTransfer2State::kCreated)
  , m_finish(false)
  , m_currentCommunicationMode( mode )
  , m_currentTimingParams( params )
  , m_sender(sender)
  , m_defaultError(defaultError)
  , m_defaultTimeout( defaultTimeout )
{
  TRC_FUNCTION_ENTER( PAR( mode ) << PAR( defaultTimeout ) << PAR( userTimeout ) )
    static uint32_t transactionId = 0;
  m_transactionId = ++transactionId;

  const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();

  int32_t requiredTimeout = userTimeout;

  // check and correct timeout here before blocking:
  if ( requiredTimeout < 0 ) {
    // Discovery or SmartConnect or Authorize or FRC command ?
    if ( ( message.NodeAddress() & BROADCAST_ADDRESS ) == COORDINATOR_ADDRESS && (
      message.DpaPacket().DpaRequestPacket_t.PCMD == CMD_COORDINATOR_DISCOVERY ||
      message.DpaPacket().DpaRequestPacket_t.PCMD == CMD_COORDINATOR_SMART_CONNECT ||
      message.DpaPacket().DpaRequestPacket_t.PCMD == CMD_COORDINATOR_AUTHORIZE_BOND ||
      message.DpaPacket().DpaRequestPacket_t.PCMD == CMD_FRC_SEND ||
      message.DpaPacket().DpaRequestPacket_t.PCMD == CMD_FRC_SEND_SELECTIVE
      ) ) {
      // Yes, set default (infinite) timeout for Discovery or SmartConnect
      TRC_WARNING( PAR( requiredTimeout ) << " Default (infinite) timeout forced for Discovery or SmartConnect or Authorize or FRC message" );
      m_infinitTimeout = true;
    }
    // default timeout
    requiredTimeout = defaultTimeout;
  }
  else if ( requiredTimeout == INFINITE_TIMEOUT ) {
    // it is allowed just for Coordinator Discovery, SmartConnect, Authorize and FRC
    if ( ( message.NodeAddress() & BROADCAST_ADDRESS ) != COORDINATOR_ADDRESS || (
      message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_COORDINATOR_DISCOVERY &&
      message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_COORDINATOR_SMART_CONNECT &&
      message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_COORDINATOR_AUTHORIZE_BOND &&
      message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_FRC_SEND &&
      message.DpaPacket().DpaRequestPacket_t.PCMD != CMD_FRC_SEND_SELECTIVE
      ) ) {
      // force setting minimal timing as only Discovery can have infinite timeout
      TRC_WARNING( "User: " << PAR( requiredTimeout ) << " forced to: " << PAR( defaultTimeout ) );
      requiredTimeout = defaultTimeout;
    }
    else {
      TRC_WARNING( PAR( requiredTimeout ) << " infinite timeout allowed for Discovery or SmartConnect or Authorize or FRC message" );
      requiredTimeout = defaultTimeout;
      m_infinitTimeout = true;
    }
  }
  else if ( requiredTimeout < defaultTimeout ) {
    TRC_WARNING( "User: " << PAR( requiredTimeout ) << " forced to: " << PAR( defaultTimeout ) );
    requiredTimeout = defaultTimeout;
  }

  // init expected duration - no estimation yet, so use default timeout
  m_expectedDurationMs = m_defaultTimeout;

  // calculate requiredTimeout for special cases
  if ( ( message.NodeAddress() & BROADCAST_ADDRESS ) == COORDINATOR_ADDRESS )
  {
    if ( requiredTimeout > defaultTimeout )
    {
      m_expectedDurationMs = requiredTimeout;
    }

    // peripheral FRC and FRC command
    //if ( message.DpaPacket().DpaRequestPacket_t.PNUM == PNUM_FRC &&
    //  ( message.PeripheralCommand() == CMD_FRC_SEND || message.PeripheralCommand() == CMD_FRC_SEND_SELECTIVE ) )
    //{
    //  // user timeout is not applied, timeout forced to FRC 
    //  requiredTimeout = getFrcTimeout();
    //  m_expectedDurationMs = requiredTimeout;
    //  TRC_WARNING( "User: " << PAR( userTimeout ) << " forced to FRC: " << PAR( requiredTimeout ) );
    //}

    //bonding special timeout 
    if ( message.DpaPacket().DpaRequestPacket_t.PNUM == PNUM_COORDINATOR &&
      ( message.PeripheralCommand() == CMD_COORDINATOR_BOND_NODE ) )
    {
      // user timeout is not applied, forced to BOND_TIMEOUT_MS
      if ( userTimeout < 0 ) {
        requiredTimeout = BOND_TIMEOUT_MS;
        m_expectedDurationMs = requiredTimeout;
        TRC_INFORMATION( "Used timeout: " << PAR( BOND_TIMEOUT_MS ) );
      }
    }
  }
  
  m_userTimeoutMs = requiredTimeout; // checked and corrected timeout
  TRC_FUNCTION_LEAVE( "Using: " << PAR( m_userTimeoutMs ) );
}

DpaTransaction2::~DpaTransaction2()
{
}

void DpaTransaction2::abort() {
  std::unique_lock<std::mutex> lck( m_conditionVariableMutex );
  m_state = kAborted;
  m_conditionVariable.notify_all();
}

//-----------------------------------------------------
std::unique_ptr<IDpaTransactionResult2> DpaTransaction2::get()
{
  // wait for transaction start
  TRC_DEBUG( "wait for start: " << PAR( m_transactionId ) );

  // lock this function except blocking in wait_for()
  std::unique_lock<std::mutex> lck( m_conditionVariableMutex );

  while ( m_infinitTimeout ) {
    // wait_for() unlock lck when blocking and lock it again when get out, waiting continue if not started (predicate == false)
    if ( !m_conditionVariable.wait_for( lck, std::chrono::milliseconds( m_userTimeoutMs ), [&] { return m_state != kCreated; } ) )
    {
      // out of wait timeout
      if ( !m_infinitTimeout ) {
        TRC_WARNING( "Transaction timeout - transaction was not started in time." );
        m_dpaTransactionResultPtr->setErrorCode( IDpaTransactionResult2::TRN_ERROR_IFACE_BUSY );
        // return result and move ownership 
        return std::move( m_dpaTransactionResultPtr );
      }
      else {
        TRC_WARNING( "Infinit timeout - wait forever." );
      }
    }
    // out of wait notify from execute() or processReceivedMessage
    break;
  }

  // transaction started
  TRC_DEBUG( "Started, wait for finish: " << PAR( m_transactionId ) );
  // wait_for() unlock lck when blocking and lock it again when get out, waiting continue if not finished (predicate == false)
  while ( !m_conditionVariable.wait_for( lck, std::chrono::milliseconds( m_userTimeoutMs ), [&] { return m_finish; } ) );

  // return result and move ownership 
  TRC_DEBUG( "Finished: " << PAR( m_transactionId ) << PAR( m_state ) );
  return std::move( m_dpaTransactionResultPtr );
}

//-----------------------------------------------------
void DpaTransaction2::execute()
{
  execute(m_defaultError);
}

//-----------------------------------------------------
void DpaTransaction2::execute(IDpaTransactionResult2::ErrorCode defaultError)
{
  // lock this function except blocking in wait_for()
  std::unique_lock<std::mutex> lck( m_conditionVariableMutex );
  
  m_defaultError = defaultError;

  const DpaMessage& message = m_dpaTransactionResultPtr->getRequest();

  if ( m_defaultError == IDpaTransactionResult2::TRN_OK) {
    // init transaction state
    if ( ( message.NodeAddress() & BROADCAST_ADDRESS ) == COORDINATOR_ADDRESS ) {
      m_state = kSentCoordinator;
    }
    else {
      m_state = kSent;
    }

    // send request toward coordinator via send functor
    try {
      m_sender( message );
      // now we can expect handling in processReceivedMessage()
    }
    catch ( std::exception& e ) {
      TRC_WARNING( "Send error occurred: " << e.what() );
      // init expected duration - we have final error state - just finish transaction
      m_expectedDurationMs = 0;
      m_state = kInterfaceError;
    }
  }
  else {
    // transaction is not handled 
    m_state = kDefaultError;
    // init expected duration - we have final error state - just finish transaction
    m_expectedDurationMs = 0;
  }

  // 1st notification to get() - we started transaction 
  m_conditionVariable.notify_one();

  int errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;
  bool finish = true;
  bool expired = false;

  do {

    finish = true; // end this cycle
    expired = false;

    // wait on conditon 
    if ( m_expectedDurationMs > 0 ) {
      // wait_for() unlock lck when blocking and lock it again when get out, processReceivedMessage() is able to do its job as it can lock now
      if ( std::cv_status::timeout == m_conditionVariable.wait_for( lck, std::chrono::milliseconds( m_expectedDurationMs ) ) ) {
        // out of wait on timeout
        expired = true;
      }
      // out of wait on notify from processReceivedMessage()
    }

    // evaluate state
    switch ( m_state )
    {
      case kSent:
      case kSentCoordinator:
      case kConfirmation:
        if ( expired ) {
          if ( !m_infinitTimeout ) {
            m_state = kTimeout;
            errorCode = DpaTransactionResult2::TRN_ERROR_TIMEOUT;
          }
          else {
            // reset finish we have to wait forever
            finish = false;
          }
        }
        else {
          // reset finish we didn't finish yet
          finish = false;
        }
        break;
      case kConfirmationBroadcast:
        if ( expired ) {
          m_state = kProcessed;
          errorCode = DpaTransactionResult2::TRN_OK;
        }
        else {
          // reset finish we didn't finish yet
          finish = false;
        }
        break;
      case kReceivedResponse:
        if ( expired ) {
          m_state = kProcessed;
          errorCode = DpaTransactionResult2::TRN_OK;
        }
        else {
          // reset finish we didn't finish yet
          finish = false;
        }
        break;
      case kProcessed:
        errorCode = DpaTransactionResult2::TRN_OK;
        break;
      case kTimeout:
        errorCode = DpaTransactionResult2::TRN_ERROR_TIMEOUT;
        break;
      case kAborted:
        errorCode = DpaTransactionResult2::TRN_ERROR_ABORTED;
        break;
      case kInterfaceError:
        errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;
        break;
      case kDefaultError:
        errorCode = m_defaultError;
        break;
      default:
        errorCode = DpaTransactionResult2::TRN_ERROR_IFACE;
    }

  } while ( !finish );

  // update error code in result
  m_dpaTransactionResultPtr->setErrorCode( errorCode );

  // signalize final state
  m_finish = true;

  // 2st notification to get() 
  m_conditionVariable.notify_one();
}

  //-----------------------------------------------------
void DpaTransaction2::processReceivedMessage( const DpaMessage& receivedMessage )
{
  TRC_FUNCTION_ENTER( "" );

  // lock this function
  std::unique_lock<std::mutex> lck( m_conditionVariableMutex );

  //check transaction state
  if ( m_finish ) {
    return; //nothing to do, just double check
  }

  DpaMessage::MessageType messageDirection = receivedMessage.MessageDirection();

  //check massage validity
  // TODO verify exception handling. Do we need the exception here at all?
  // no request is expected
  if ( messageDirection != DpaMessage::kResponse && messageDirection != DpaMessage::kConfirmation ) {
    throw std::logic_error( "Response is expected." );
  }
  const DpaMessage& request = m_dpaTransactionResultPtr->getRequest();
  // same as sent request
  if ( receivedMessage.NodeAddress() != request.NodeAddress() ) {
    throw std::logic_error( "Different node address than in sent message." );
  }
  // same as sent request
  if ( receivedMessage.PeripheralType() != request.PeripheralType() ) {
    throw std::logic_error( "Different peripheral type than in sent message." );
  }
  // same as sent request
  if ( ( receivedMessage.PeripheralCommand() & ~0x80 ) != request.PeripheralCommand() ) {
    throw std::logic_error( "Different peripheral command than in sent message." );
  }

  int32_t estimatedTimeMs = 0;

  // process confirmation
  if ( messageDirection == DpaMessage::kConfirmation ) {
    if (( receivedMessage.NodeAddress() & BROADCAST_ADDRESS )== BROADCAST_ADDRESS ) {
      m_state = kConfirmationBroadcast;
    }
    else {
      m_state = kConfirmation;
    }

    // setting timeout based on the confirmation
    TIFaceConfirmation iFace = receivedMessage.DpaPacket().DpaResponsePacket_t.DpaMessage.IFaceConfirmation;

    // save for later use with response
    m_hops = iFace.Hops;
    m_timeslotLength = iFace.TimeSlotLength;
    m_hopsResponse = iFace.HopsResponse;

    if ( m_currentCommunicationMode == RfMode::kLp ) {
      estimatedTimeMs = EstimateLpTimeout( m_hops, m_timeslotLength, m_hopsResponse );
    }
    else { // std
      estimatedTimeMs = EstimateStdTimeout( m_hops, m_timeslotLength, m_hopsResponse );
    }

    if ( estimatedTimeMs > 0 ) {
      TRC_INFORMATION( "Expected duration to wait :" << PAR( m_userTimeoutMs ) << PAR( estimatedTimeMs ) );
      if (static_cast<uint32_t>(estimatedTimeMs) >= m_userTimeoutMs ) {
        m_expectedDurationMs = estimatedTimeMs;
      }
      else {
        // user wants to wait more then estimated so keep the wish
        m_expectedDurationMs = m_userTimeoutMs;
      }
    }

    TRC_DEBUG( "From confirmation: " << PAR( estimatedTimeMs ) );

    m_dpaTransactionResultPtr->setConfirmation( receivedMessage );
    TRC_INFORMATION( "Confirmation processed." );
  }

  // process response
  else {
    // if there was a request to coordinator then after receiving response it is allowed to send another
    if ( m_state == kSentCoordinator ) {
      // done, next request gets ready 
      m_state = kProcessed;
    }
    else {
      // only if there is not infinite timeout
      if ( !m_infinitTimeout ) {
        m_state = kReceivedResponse;
        /////////////////////////////
        // TODO is it necessary here to wait if we have the response already?
        // or is it aditional refresh timeout for some reason depending on response len?
        if ( m_currentCommunicationMode == RfMode::kLp ) {
          estimatedTimeMs = EstimateLpTimeout(static_cast<uint8_t>(m_hops), static_cast<uint8_t>(m_timeslotLength), static_cast<uint8_t>(m_hopsResponse),
            static_cast<int8_t>(receivedMessage.GetLength() - ( sizeof( TDpaIFaceHeader ) + 2 )) );
        }
        else { // std
          estimatedTimeMs = EstimateStdTimeout(static_cast<uint8_t>(m_hops), static_cast<uint8_t>(m_timeslotLength), static_cast<uint8_t>(m_hopsResponse),
            static_cast<int8_t>(receivedMessage.GetLength() - (sizeof(TDpaIFaceHeader) + 2)));
        }
        TRC_DEBUG( "From response: " << PAR( estimatedTimeMs ) );
        m_expectedDurationMs = estimatedTimeMs;
        if ( m_expectedDurationMs <= 0 ) {
          m_state = kProcessed;
        }
        /////////////////////////////
      }
      // infinite timeout
      else {
        // done, next request gets ready 
        m_state = kProcessed; // TODO If the refresh timeout is necessary would be here as well?
      }
    }

    m_dpaTransactionResultPtr->setResponse( receivedMessage );
    TRC_INFORMATION( "Response processed." );
  }

  // notification to execute() and get()
  m_conditionVariable.notify_all();

  TRC_FUNCTION_LEAVE( "" );
}

  // TODO it is not necessary pass the values as they are stored in members
  // m_hops, m_timeslotLength, m_hopsResponse,
  // we will need other network structure info for FRC evaluation
int32_t DpaTransaction2::EstimateStdTimeout( uint8_t hopsRequest, uint8_t timeslotReq, uint8_t hopsResponse, int8_t responseDataLength )
{
  TRC_FUNCTION_ENTER( PAR((int)hopsRequest) << PAR((int)timeslotReq) << PAR((int)hopsResponse) << PAR((int)responseDataLength) );
  int32_t responseTimeSlotLengthMs;

  auto estimatedTimeoutMs = ( hopsRequest + 1 ) * timeslotReq * 10;

  // estimation from confirmation 
  if ( responseDataLength == -1 ) {
    if ( timeslotReq == 20 )
      responseTimeSlotLengthMs = 200;
    else
      // worst case
      responseTimeSlotLengthMs = 60;
  }
  // correction of the estimation from response 
  else {
    TRC_DEBUG( "PData length of the received response: " << PAR( (int)responseDataLength ) );
    if ( m_currentTimingParams.osVersion == "4.03D" ) {
      // OS 4.03D
      if( responseDataLength < 17)
        responseTimeSlotLengthMs = 40;
      else if ( responseDataLength < 41 )
        responseTimeSlotLengthMs = 50;
      else
        responseTimeSlotLengthMs = 60;
    }
    else {
      // OS 4.02D (default)
      if ( responseDataLength < 16 )
        responseTimeSlotLengthMs = 40;
      else if ( responseDataLength < 40 )
        responseTimeSlotLengthMs = 50;
      else
        responseTimeSlotLengthMs = 60;
    }
    TRC_DEBUG( "Correction of the response timeout: " << PAR( responseTimeSlotLengthMs ) );
  }

  estimatedTimeoutMs += ( hopsResponse + 1 ) * responseTimeSlotLengthMs /* * 10 */ + SAFETY_TIMEOUT_MS;

  TRC_INFORMATION( "Estimated STD timeout: " << PAR( estimatedTimeoutMs ) );
  TRC_FUNCTION_LEAVE( "" );
  return estimatedTimeoutMs;
}

int32_t DpaTransaction2::EstimateLpTimeout( uint8_t hopsRequest, uint8_t timeslotReq, uint8_t hopsResponse, int8_t responseDataLength )
{
  TRC_FUNCTION_ENTER( PAR((int)hopsRequest) << PAR((int)timeslotReq) << PAR((int)hopsResponse) << PAR((int)responseDataLength) );
  int32_t responseTimeSlotLengthMs;

  auto estimatedTimeoutMs = ( hopsRequest + 1 ) * timeslotReq * 10;

  // estimation from confirmation 
  if ( responseDataLength == -1 ) {
    if ( timeslotReq == 20 ) {
      responseTimeSlotLengthMs = 200;
    }
    else {
      // worst case
      responseTimeSlotLengthMs = 110;
    }
  }
  // correction of the estimation from response 
  else {
    TRC_DEBUG( "PData length of the received response: " << PAR( (int)responseDataLength ) );
    if ( m_currentTimingParams.osVersion == "4.03D" ) {
      // OS 4.03D
      if ( responseDataLength < 17 )
        responseTimeSlotLengthMs = 80;
      else if ( responseDataLength < 41)
        responseTimeSlotLengthMs = 90;
      else
        responseTimeSlotLengthMs = 100;
    }
    else {
      // OS 4.02D (default)
      if ( responseDataLength < 11 )
        responseTimeSlotLengthMs = 80;
      else if ( responseDataLength < 34 )
        responseTimeSlotLengthMs = 90;
      else if ( responseDataLength < 57 )
        responseTimeSlotLengthMs = 100;
      else
        responseTimeSlotLengthMs = 110;
    }
    TRC_DEBUG( "Correction of the response timeout: " << PAR( responseTimeSlotLengthMs ) );
  }

  estimatedTimeoutMs += ( hopsResponse + 1 ) * responseTimeSlotLengthMs /* * 10 */ + SAFETY_TIMEOUT_MS;

  TRC_INFORMATION( "Estimated LP timeout: " << PAR( estimatedTimeoutMs ) );
  TRC_FUNCTION_LEAVE( "" );
  return estimatedTimeoutMs;
}

int32_t DpaTransaction2::getFrcTimeout()
{
  uint32_t timeout = m_defaultTimeout;
  uint32_t FrcResponseTime;

  // set FRC response time
  switch ( m_currentTimingParams.frcResponseTime ) {
    case IDpaTransaction2::FrcResponseTime::k360Ms:
      FrcResponseTime = 360;
      break;

    case IDpaTransaction2::FrcResponseTime::k680Ms:
      FrcResponseTime = 680;
      break;

    case IDpaTransaction2::FrcResponseTime::k1320Ms:
      FrcResponseTime = 1320;
      break;

    case IDpaTransaction2::FrcResponseTime::k2600Ms:
      FrcResponseTime = 2600;
      break;

    case IDpaTransaction2::FrcResponseTime::k5160Ms:
      FrcResponseTime = 5160;
      break;

    case IDpaTransaction2::FrcResponseTime::k10280Ms:
      FrcResponseTime = 10280;
      break;

    case IDpaTransaction2::FrcResponseTime::k20620Ms:
      FrcResponseTime = 20620;
      break;

    case IDpaTransaction2::FrcResponseTime::k40Ms:
    default:
      FrcResponseTime = 40;
      break;
  }

  if ( m_currentCommunicationMode == RfMode::kStd )
    // STD mode Advanced FRC
    timeout = m_currentTimingParams.bondedNodes * 30 + ( m_currentTimingParams.discoveredNodes + 2 ) * 110 + FrcResponseTime + 220;
  else
    // LP mode Advanced FRC
    timeout = m_currentTimingParams.bondedNodes * 30 + ( m_currentTimingParams.discoveredNodes + 2 ) * 160 + FrcResponseTime + 260;

  return timeout;
}
