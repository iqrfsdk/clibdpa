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

#include "IDpaTransaction2.h"
#include "DpaTransactionResult2.h"
#include "DpaMessage.h"
#include <condition_variable>
#include <memory>

class DpaTransaction2 : public IDpaTransaction2
{
public:
  /// type of functor to send the request message towards the coordinator
  typedef std::function<void( const DpaMessage& dpaMessage )> SendDpaMessageFunc;
  DpaTransaction2() = delete;
  DpaTransaction2( const DpaMessage& request,
    RfMode mode, TimingParams params, int32_t defaultTimeout, int32_t userTimeout, SendDpaMessageFunc sender,
    IDpaTransactionResult2::ErrorCode defaultError);
  virtual ~DpaTransaction2();
  void abort() override;
  std::unique_ptr<IDpaTransactionResult2> get();
  void execute();
  void execute(IDpaTransactionResult2::ErrorCode defaultError);
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
    kInterfaceError,
    /// error state enforced at the beginning of the transaction
    kDefaultError
  };

  /// Result object tobe returned when the transaction finishes
  std::unique_ptr<DpaTransactionResult2> m_dpaTransactionResultPtr;

  /// transaction state
  DpaTransfer2State m_state = DpaTransfer2State::kCreated;
  /// signalize final state
  bool m_finish = false;

  /// actual communication mode
  RfMode m_currentCommunicationMode;

  /// actual timing params
  TimingParams m_currentTimingParams;

  /// functor to send the request message towards the coordinator
  SendDpaMessageFunc m_sender;

  IDpaTransactionResult2::ErrorCode m_defaultError = IDpaTransactionResult2::TRN_OK;
  uint32_t m_defaultTimeout = DEFAULT_TIMEOUT; //set form configuration
  uint32_t m_userTimeoutMs = DEFAULT_TIMEOUT; //required by user
  uint32_t m_expectedDurationMs = DEFAULT_TIMEOUT;
  bool m_infinitTimeout = false;

  /// iqrf structure info to estimate transaction processing time
  int8_t m_hops = 0;
  int8_t m_timeslotLength = 0;
  int8_t m_hopsResponse = 0;

  TimingParams m_FRC_TimingParams;

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
