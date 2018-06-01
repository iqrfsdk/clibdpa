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

#include "DpaMessage.h"
#include "DpaTransactionResult2.h"
#include <future>

class IDpaTransaction2
{
public:
  enum RfMode {
    kStd,
    kLp
  };

  enum FrcResponseTime {
    k40Ms = 0x00,
    k360Ms = 0x10,
    k680Ms = 0x20,
    k1320Ms = 0x30,
    k2600Ms = 0x40,
    k5160Ms = 0x50,
    k10280Ms = 0x60,
    k20620Ms = 0x70
  };

  typedef struct
  {
    uint8_t bondedNodes;
    uint8_t discoveredNodes;
    FrcResponseTime responseTime;
  } FRC_TimingParams;

  // Timing constants
  /// Default timeout
  static const int DEFAULT_TIMEOUT = 500;
  /// Minimal timeout used if required by user is too low
  static const int MINIMAL_TIMEOUT = 200;
  /// Zero value used to indicate infinit timeout in special cases (discovery)
  static const int INFINITE_TIMEOUT = 0;
  /// An extra timeout added to timeout from a confirmation packet.
  static const int32_t SAFETY_TIMEOUT_MS = 40;

  virtual ~IDpaTransaction2() {}
  /// wait for result
  virtual std::unique_ptr<IDpaTransactionResult2> get() = 0;
  /// abort the transaction immediately 
  virtual void abort() = 0;
};

class DpaTransaction2 : public IDpaTransaction2
{
public:
  /// type of functor to send the request message towards the coordinator
  typedef std::function<void( const DpaMessage& dpaMessage )> SendDpaMessageFunc;
  DpaTransaction2() = delete;
  DpaTransaction2( const DpaMessage& request, RfMode mode, FRC_TimingParams params, int32_t defaultTimeout, int32_t userTimeout, SendDpaMessageFunc sender );
  virtual ~DpaTransaction2();
  void abort() override;
  std::unique_ptr<IDpaTransactionResult2> get();
  void execute( bool queued );
  void processReceivedMessage( const DpaMessage& receivedMessage );

private:
  //// Values that represent transaction state.
  enum DpaTransfer2State
  {
    /// init state
    kCreated,
    /// request was sent.
    kSent,
    /// request with coordinator address was sent.
    kSentCoordinator,
    /// confirmation was received.
    kConfirmation,
    /// confirmation broadcast was received.
    kConfirmationBroadcast,
    /// response was received wait for expected time
    kReceivedResponse,
    /// transaction was processed.
    kProcessed,
    /// timeout expired.
    kTimeout,
    /// transaction forced to abort.
    kAborted,
    /// error state during sending via iqrf interface.
    kError,
    /// iqrf interface queue is full
    kQueueFull
  };

  /// Result object tobe returned when the transaction finishes
  std::unique_ptr<DpaTransactionResult2> m_dpaTransactionResultPtr;

  /// transaction state
  DpaTransfer2State m_state = kCreated;
  /// signalize final state
  bool m_finish = false;

  /// actual communication mode
  RfMode m_currentCommunicationMode;

  /// actual FRC timing params
  FRC_TimingParams m_currentFRC_TimingParams;

  /// functor to send the request message towards the coordinator
  SendDpaMessageFunc m_sender;

  uint32_t m_defaultTimeout = DEFAULT_TIMEOUT; //set form configuration
  uint32_t m_userTimeoutMs = DEFAULT_TIMEOUT; //required by user
  uint32_t m_expectedDurationMs = DEFAULT_TIMEOUT;
  bool m_infinitTimeout = false;

  /// iqrf structure info to estimate transaction processing time
  int8_t m_hops = 0;
  int8_t m_timeslotLength = 0;
  int8_t m_hopsResponse = 0;

  FRC_TimingParams m_FRC_TimingParams;

  /// condition used to wait for confirmation and response messages from coordinator
  std::condition_variable m_conditionVariable;
  /// mutex to protect shared variables controlled by condition variable
  std::mutex m_conditionVariableMutex;

  /// internal transaction ID
  uint32_t m_transactionId;

  // TODO it is not necessary pass the values as they are stored in members
  // m_hops, m_timeslotLength, m_hopsResponse,
  // we will need other network structure info for FRC evaluation
  int32_t EstimateStdTimeout( uint8_t hopsRequest, uint8_t timeslotReq, uint8_t hopsResponse, int8_t responseDataLength = -1 );
  int32_t EstimateLpTimeout( uint8_t hopsRequest, uint8_t timeslotReq, uint8_t hopsResponse, int8_t responseDataLength = -1 );
  int32_t getFrcTimeout();
};