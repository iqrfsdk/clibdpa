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
#include "IqrfTraceHex.h"
#include "IChannel.h"
#include <exception>
#include <future>

/////////////////////////////////////
// class DpaHandler2::Imp
/////////////////////////////////////
class DpaHandler2::Imp
{
public:
  // maximal queue lenght - can be prolonged if it make sense
  static const int QUEUE_MAX_LEN = 16;

  Imp( IChannel* iqrfInterface )
    :m_iqrfInterface( iqrfInterface )
  {
    m_dpaTransactionQueue = ant_new TaskQueue<std::shared_ptr<DpaTransaction2>>( [&]( std::shared_ptr<DpaTransaction2> ptr ) {
      m_pendingTransaction = ptr;
      size_t size = m_dpaTransactionQueue->size();
      if ( size < QUEUE_MAX_LEN ) {
        m_pendingTransaction->execute(); // succesfully queued
      }
      else {
        TRC_ERROR( "Transaction queue overload: " << PAR( size ) );
        m_pendingTransaction->execute(IDpaTransactionResult2::TRN_ERROR_IFACE_QUEUE_FULL);  // queue full transaction not handled, error reported
      }
    } );

    if ( iqrfInterface == nullptr ) {
      throw std::invalid_argument( "DPA interface argument can not be nullptr." );
    }
    m_iqrfInterface = iqrfInterface;

    // register callback for cdc or spi interface
    m_iqrfInterface->registerReceiveFromHandler( [&]( const std::basic_string<unsigned char>& msg ) -> int {
      ResponseMessageHandler( msg );
      return 0;
    } );

    // initialize m_FrcTimingParams 
    m_timingParams.bondedNodes = 1;
    m_timingParams.discoveredNodes = 1;
    m_timingParams.osVersion = "4.02D";
    m_timingParams.dpaVersion = 0x0302;
    m_timingParams.frcResponseTime = IDpaTransaction2::FrcResponseTime::k40Ms;
  }

  ~Imp()
  {
    // kill DpaTransaction if any
    if ( m_pendingTransaction ) {
      m_pendingTransaction->abort();
    }
    m_dpaTransactionQueue->stopQueue();
    delete m_dpaTransactionQueue;
  }

  // any received message from the channel
  void ResponseMessageHandler( const std::basic_string<unsigned char>& message ) {
    if ( message.length() == 0 )
      return;

    TRC_INFORMATION( ">>>>>>>>>>>>>>>>>>" << std::endl <<
             "Received from IQRF interface: " << std::endl << MEM_HEX( message.data(), message.length() ) );

    // incomming message
    DpaMessage receivedMessage;
    try {
      receivedMessage.FillFromResponse( message.data(), message.length() );
    }
    catch ( std::exception& e ) {
      CATCH_EXC_TRC_WAR(std::exception, e, "in processing msg");
      return;
    }

    auto messageDirection = receivedMessage.MessageDirection();
    if ( messageDirection == DpaMessage::MessageType::kRequest ) {
      //Always Async
      processAsynchronousMessage( message );
      return;
    }
    else if ( messageDirection == DpaMessage::MessageType::kResponse &&
              receivedMessage.DpaPacket().DpaResponsePacket_t.ResponseCode & STATUS_ASYNC_RESPONSE ) {
      // async msg
      processAsynchronousMessage( message );
      return;
    }
    else {
      try {
        m_pendingTransaction->processReceivedMessage( message );
      }
      catch ( std::logic_error& le ) {
        CATCH_EXC_TRC_WAR(std::logic_error, le, "Process received message error..." );
      }
    }
  }

  std::shared_ptr<IDpaTransaction2> executeDpaTransaction( const DpaMessage& request, int32_t timeout, 
    IDpaTransactionResult2::ErrorCode defaultError)
  {
    if ( request.GetLength() <= 0 ) {
      //TODO gets stuck on DpaTransaction2::get() if processed here
      TRC_WARNING( "Empty request => nothing to sent and transaction aborted" );
      std::shared_ptr<DpaTransaction2> ptr( ant_new DpaTransaction2( request, m_rfMode, m_timingParams, m_defaultTimeout, timeout, nullptr, defaultError ) );
      return ptr;
    }
    std::shared_ptr<DpaTransaction2> ptr( ant_new DpaTransaction2( request,
      m_rfMode, m_timingParams, m_defaultTimeout, timeout,
      [&]( const DpaMessage& r ) {
        sendRequest( r );
      },
      defaultError
    ));
    m_dpaTransactionQueue->pushToQueue( ptr );
    return ptr;
  }

  int getTimeout() const
  {
    return m_defaultTimeout;
  }

  void setTimeout( int timeout )
  {
    if ( timeout < IDpaTransaction2::MINIMAL_TIMEOUT ) {
      TRC_WARNING( PAR( timeout ) << " is too low and it is forced to: " << PAR( IDpaTransaction2::MINIMAL_TIMEOUT ) )
        timeout = IDpaTransaction2::MINIMAL_TIMEOUT;
    }
    m_defaultTimeout = timeout;
  }

  IDpaTransaction2::RfMode getRfCommunicationMode() const
  {
    return m_rfMode;
  }

  void setRfCommunicationMode( IDpaTransaction2::RfMode rfMode )
  {
    //TODO set rfMode on iqrf interface
    m_rfMode = rfMode;
  }

  IDpaTransaction2::TimingParams getTimingParams() const
  {
    return m_timingParams;
  }

  void setTimingParams( IDpaTransaction2::TimingParams params )
  {
    m_timingParams = params;
  }

  IDpaTransaction2::FrcResponseTime getFrcResponseTime() const
  {
    return m_timingParams.frcResponseTime;
  }

  void setFrcResponseTime( IDpaTransaction2::FrcResponseTime frcResponseTime )
  {
    m_timingParams.frcResponseTime = frcResponseTime;
  }

  void registerAsyncMessageHandler( const std::string& serviceId, AsyncMessageHandlerFunc fun )
  {
    //TODO serviceId?
    std::lock_guard<std::mutex> lck( m_asyncMessageMutex );
    m_asyncMessageHandler = fun;
  }

  void processAsynchronousMessage( const DpaMessage& message ) {
    m_asyncMessageMutex.lock();

    if ( m_asyncMessageHandler ) {
      m_asyncMessageHandler( message );
    }

    m_asyncMessageMutex.unlock();
  }

  void unregisterAsyncMessageHandler( const std::string& serviceId )
  {
    //TODO serviceId?
    std::lock_guard<std::mutex> lck( m_asyncMessageMutex );
    m_asyncMessageHandler = nullptr;
  }

private:
  void sendRequest( const DpaMessage& request )
  {
    TRC_INFORMATION( "<<<<<<<<<<<<<<<<<<" << std::endl <<
             "Sent to DPA interface: " << std::endl << MEM_HEX( request.DpaPacketData(), request.GetLength() ) );
    m_iqrfInterface->sendTo( std::basic_string<unsigned char>( request.DpaPacketData(), request.GetLength() ) );
  }

  IDpaTransaction2::RfMode m_rfMode = IDpaTransaction2::RfMode::kStd;
  IDpaTransaction2::TimingParams m_timingParams;

  AsyncMessageHandlerFunc m_asyncMessageHandler;
  std::mutex m_asyncMessageMutex;
  IChannel* m_iqrfInterface = nullptr;
  int m_defaultTimeout = IDpaTransaction2::DEFAULT_TIMEOUT;

  std::shared_ptr<DpaTransaction2> m_pendingTransaction;
  TaskQueue<std::shared_ptr<DpaTransaction2>>* m_dpaTransactionQueue = nullptr;
};

/////////////////////////////////////
// class DpaHandler2
/////////////////////////////////////
DpaHandler2::DpaHandler2( IChannel* iqrfInterface )
{
  m_imp = ant_new Imp( iqrfInterface );
}

DpaHandler2::~DpaHandler2()
{
  delete m_imp;
}

std::shared_ptr<IDpaTransaction2> DpaHandler2::executeDpaTransaction( const DpaMessage& request, int32_t timeout,
  IDpaTransactionResult2::ErrorCode defaultError)
{
  return m_imp->executeDpaTransaction( request, timeout, defaultError );
}

int DpaHandler2::getTimeout() const
{
  return m_imp->getTimeout();
}

void DpaHandler2::setTimeout( int timeout )
{
  m_imp->setTimeout( timeout );
}

IDpaTransaction2::RfMode DpaHandler2::getRfCommunicationMode() const
{
  return m_imp->getRfCommunicationMode();
}

void DpaHandler2::setRfCommunicationMode( IDpaTransaction2::RfMode rfMode )
{
  m_imp->setRfCommunicationMode( rfMode );
}

IDpaTransaction2::TimingParams DpaHandler2::getTimingParams() const
{
  return m_imp->getTimingParams();
}

void DpaHandler2::setTimingParams( IDpaTransaction2::TimingParams params )
{
  m_imp->setTimingParams( params );
}

IDpaTransaction2::FrcResponseTime DpaHandler2::getFrcResponseTime() const
{
  return m_imp->getFrcResponseTime();
}

void DpaHandler2::setFrcResponseTime( IDpaTransaction2::FrcResponseTime frcResponseTime )
{
  m_imp->setFrcResponseTime( frcResponseTime );
}

void DpaHandler2::registerAsyncMessageHandler( const std::string& serviceId, IDpaHandler2::AsyncMessageHandlerFunc fun )
{
  m_imp->registerAsyncMessageHandler( serviceId, fun );
}

void DpaHandler2::unregisterAsyncMessageHandler( const std::string& serviceId )
{
  m_imp->unregisterAsyncMessageHandler( serviceId );
}
