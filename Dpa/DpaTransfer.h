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

#include <chrono>
#include <mutex>

class DpaTransaction;

class DpaTransfer
{
public:
  /** Values that represent IQRF communication modes. */
  enum IqrfRfCommunicationMode
  {
    kStd,
    kLp
  };

  /** Values that represent DPA transfer state. */
  enum DpaTransferStatus
  {
    kCreated,
    ///< An enum constant representing the first message was sent.
    kSent,
    ///< An enum constant representing the confirmation was received.
    kConfirmation,
    kConfirmationBroadcast,
    ///< An enum constant representing the timeout expired.
    kTimeout,
    ///< An enum constant representing the whole transfer was processed.
    kProcessed,
    kReceivedResponse,
    ///< An enum constant representing the transfer was aborted.
    kAborted,
    ///< An enum constant representing the error state during sending via iqrf interface.
    kError
  };

  /** Default constructor */
  DpaTransfer();

  /** Ctor with external response handler */
  DpaTransfer(DpaTransaction* dpaTransaction);

  /** Destructor */
  virtual ~DpaTransfer();

  /**
   Processes message sent to network, stores it for future uses and sets status. Only one message per
   request could be sent.

   @exception	std::logic_error	Raised when status is not kCreated.

   @param	sent_message	The message which was sent.
   */
  void ProcessSentMessage(const DpaMessage& sentMessage);

  /**
   Processes message received from the network.

   @exception	unexpected_packet_type	Thrown when a message is not a confirmation or response type.
   @exception	unexpected_peripheral 	Thrown when a message is not from the same peripheral like sent message.
   @exception	unexpected_command	  	Thrown when a message is not the response for command from sent message.

   @param	received_message	Received message.
   */
  void ProcessReceivedMessage(const DpaMessage& receivedMessage);

  /**
   Gets a sent message.

   @return	A reference to the sent message. Otherwise returns nullptr if no sent message was processed.
   */
  const DpaMessage& SentMessage() const
  {
    return *m_sentMessage;
  }

  /**
   Gets a response message.

   @return	A reference to the received message. Otherwise returns nullptr if no sent message was processed.
   */
  const DpaMessage& ResponseMessage() const
  {
    return *m_responseMessage;
  }

  /**
   Gets the status of the DPA transfer.

   @return	The status of the message.
   */
  DpaTransferStatus ProcessStatus();

  /**
  Aborts pending request
  */
  void Abort();

  /**
   Query if request is in progress state.

   @return	true if is in progress, false if not.
  */
  bool IsInProgress();

  /**
 Query if request is in progress state.

 @param [in,out]	expectedDuration	Expected time to finish DPA transaction.
 @return	true if is in progress, false if not.
*/
  bool IsInProgress(int32_t& expectedDuration);

  /**
   Gets timeout in ms.

   @return	Timeout in ms.
   */
  int32_t DefaultTimeout() const
  {
    return m_timeoutMs;
  }

  /**
   Sets default timeout.

   @param	timeoutMs	Timeout in milliseconds.
   */
  void DefaultTimeout(int32_t timeoutMs)
  {
    this->m_timeoutMs = timeoutMs;
  }

  /**
  Estimated timeout calculated for confirmation message.

  @param	confirmationMessage	The confirmation DPA message.
  @return	Estimated timeout in ms.
  */
  int32_t EstimatedTimeout(const DpaMessage& confirmationMessage);

  /**
  Set and Get IQRF communication mode.

  @param	IqrfRfCommunicationMode	mode.
  @return	IqrfRfCommunicationMode	mode.
  */
  void SetIqrfRfMode(IqrfRfCommunicationMode mode);
  IqrfRfCommunicationMode GetIqrfRfMode() const;

protected:
  /** An extra timeout added to timeout from a confirmation packet. */
  const int32_t m_safetyTimeoutMs = 40;

  virtual int32_t EstimateStdTimeout(uint8_t hops, uint8_t hopsResponse, uint8_t timeslot, int32_t response = -1);
  virtual int32_t EstimateLpTimeout(uint8_t hops, uint8_t hopsResponse, uint8_t timeslot, int32_t response = -1);

private:
  std::mutex m_statusMutex;
  IqrfRfCommunicationMode m_currentCommunicationMode;

  std::chrono::system_clock::time_point m_startTime;
  int32_t m_timeoutMs;
  int32_t m_expectedDurationMs;

  DpaTransferStatus m_status;
  DpaMessage* m_sentMessage;
  DpaMessage* m_responseMessage;
  DpaTransaction* m_dpaTransaction;

  static bool IsInProgressStatus(DpaTransferStatus status);
  void SetStatus(DpaTransferStatus status);

  void SetTimeoutForCurrentTransfer(int32_t estimatedTimeMs = 0);
  int32_t CheckTimeout();

  void ProcessConfirmationMessage(const DpaMessage& confirmationMessage);
  void ProcessResponseMessage(const DpaMessage& responseMessage);
};
