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

#include "DpaTransfer.h"
#include "DpaTransaction.h"
#include "unexpected_packet_type.h"
#include "unexpected_command.h"
#include "unexpected_peripheral.h"

DpaTransfer::DpaTransfer()
  : m_status(kCreated), m_sentMessage(nullptr), m_responseMessage(nullptr),
  m_expectedDurationMs(0), m_timeoutMs(-1), m_currentCommunicationMode(kStd), m_dpaTransaction(nullptr)
{
}

DpaTransfer::DpaTransfer(DpaTransaction* dpaTransaction)
  : m_status(kCreated), m_sentMessage(nullptr), m_responseMessage(nullptr),
  m_expectedDurationMs(0), m_timeoutMs(-1), m_currentCommunicationMode(kStd), m_dpaTransaction(dpaTransaction)
{
}

DpaTransfer::~DpaTransfer()
{
  delete m_sentMessage;
  delete m_responseMessage;
}

void DpaTransfer::ProcessSentMessage(const DpaMessage& sentMessage)
{
  if (m_status != kCreated)
  {
    throw std::logic_error("Sent message already set.");
  }

  // current request status is set as sent
  SetStatus(kSent);

  // message itself is destroyed after being sent
  delete m_sentMessage;
  // creating object to hold new request
  m_sentMessage = new DpaMessage(sentMessage);

  // setting default timeout
  SetTimeoutForCurrentTransfer();
}

void DpaTransfer::ProcessReceivedMessage(const DpaMessage& receivedMessage)
{
  if (receivedMessage.MessageDirection() != DpaMessage::kResponse
    && receivedMessage.MessageDirection() != DpaMessage::kConfirmation)
    throw unexpected_packet_type("Response is expected.");

  std::lock_guard<std::mutex> lck(m_statusMutex);

  // checks
  if (!IsInProgressStatus(m_status)) {
    throw unexpected_packet_type("Request has not been sent, yet.");
  }

  if (receivedMessage.PeripheralType() != m_sentMessage->PeripheralType()) {
    throw unexpected_peripheral("Different peripheral type than in sent message.");
  }

  if ((receivedMessage.CommandCode() & ~0x80) != m_sentMessage->CommandCode()) {
    throw unexpected_command("Different peripheral command than in sent message.");
  }

  // direction
  auto messageDirection = receivedMessage.MessageDirection();
  if (messageDirection == DpaMessage::kConfirmation) {
    if (m_dpaTransaction) {
      m_dpaTransaction->processConfirmationMessage(receivedMessage);
    }

    ProcessConfirmationMessage(receivedMessage);
  }
  else {
    if (m_dpaTransaction) {
      m_dpaTransaction->processResponseMessage(receivedMessage);
    }

    ProcessResponseMessage(receivedMessage);
  }
}

void DpaTransfer::ProcessConfirmationMessage(const DpaMessage& confirmationPacket)
{
  if (confirmationPacket.NodeAddress() == DpaMessage::kBroadCastAddress) {
    m_status = kConfirmationBroadcast;
  }
  else {
    m_status = kConfirmation;
  }

  // setting timeout based on the confirmation
  SetTimeoutForCurrentTransfer(EstimatedTimeout(confirmationPacket));
}

void DpaTransfer::ProcessResponseMessage(const DpaMessage& responseMessage)
{
  m_status = kReceivedResponse;

  delete m_responseMessage;
  m_responseMessage = new DpaMessage(responseMessage);
}

int32_t DpaTransfer::EstimatedTimeout(const DpaMessage& confirmationMessage)
{
  if (confirmationMessage.MessageDirection() != DpaMessage::kConfirmation)
    throw std::invalid_argument("Parameter is not a confirmation message.");

  auto iFace = confirmationMessage.DpaPacket().DpaResponsePacket_t.DpaMessage.IFaceConfirmation;

  if (m_currentCommunicationMode == kLp) {
    return EstimateLpTimeout(iFace.Hops, iFace.HopsResponse, iFace.TimeSlotLength);
  }
  return EstimateStdTimeout(iFace.Hops, iFace.HopsResponse, iFace.TimeSlotLength);
}

int32_t DpaTransfer::EstimateStdTimeout(uint8_t hops, uint8_t hopsResponse, uint8_t timeslot, int32_t response)
{
  auto estimatedTimeoutMs = (hops + 1) * timeslot * 10;

  int32_t responseTimeSlotLengthMs;
  if (timeslot == 20) {
    responseTimeSlotLengthMs = 200;
  }
  else {
    responseTimeSlotLengthMs = 60;
  }

  estimatedTimeoutMs += (hopsResponse + 1) * responseTimeSlotLengthMs + m_safetyTimeoutMs;
  return estimatedTimeoutMs;
}

int32_t DpaTransfer::EstimateLpTimeout(uint8_t hops, uint8_t hopsResponse, uint8_t timeslot, int32_t response)
{
  auto estimatedTimeoutMs = (hops + 1) * timeslot * 10;

  int32_t responseTimeSlotLengthMs;
  if (timeslot == 20) {
    responseTimeSlotLengthMs = 200;
  }
  else {
    responseTimeSlotLengthMs = 110;
  }

  estimatedTimeoutMs += (hopsResponse + 1) * responseTimeSlotLengthMs + m_safetyTimeoutMs;
  return estimatedTimeoutMs;
}

void DpaTransfer::SetTimeoutForCurrentTransfer(int32_t estimatedTimeMs)
{
  // infinite expected time
  if (m_status != kConfirmationBroadcast && m_timeoutMs == -1)
  {
    m_expectedDurationMs = m_timeoutMs;
    return;
  }

  m_startTime = std::chrono::system_clock::now();
  m_expectedDurationMs = m_timeoutMs + estimatedTimeMs;
}

int32_t DpaTransfer::CheckTimeout()
{
  int32_t remains(0);

  if (m_status == kProcessed || m_status == kConfirmationBroadcast || m_status == kReceivedResponse) {
    SetStatus(kProcessed);
    return remains;
  }
  else if (m_status == kAborted) {
    return remains;
  }

  bool timeout(false);

  if (m_expectedDurationMs != -1) {
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_startTime);
    remains = m_expectedDurationMs - duration.count();
    // already over?
    timeout = remains < 0;
  }

  // yes over
  if (timeout) {
    if (m_status == kConfirmationBroadcast || m_status == kReceivedResponse)
      SetStatus(kProcessed);
    else
      SetStatus(kTimeout);
  }

  // time to wait yet
  return remains;
}

bool DpaTransfer::IsInProgress() {
  return IsInProgressStatus(ProcessStatus());
}

bool DpaTransfer::IsInProgress(int32_t& expectedDuration) {
  std::lock_guard<std::mutex> lck(m_statusMutex);

  expectedDuration = CheckTimeout();
  return IsInProgressStatus(m_status);
}

DpaTransfer::DpaTransferStatus DpaTransfer::ProcessStatus() {
  std::lock_guard<std::mutex> lck(m_statusMutex);

  // changes m_status
  CheckTimeout();
  return m_status;
}

void DpaTransfer::Abort() {
  std::lock_guard<std::mutex> lck(m_statusMutex);

  m_status = kAborted;
}

DpaTransfer::IqrfRfCommunicationMode DpaTransfer::GetIqrfRfMode() const
{
  return m_currentCommunicationMode;
}

void DpaTransfer::SetIqrfRfMode(IqrfRfCommunicationMode mode)
{
  m_currentCommunicationMode = mode;
}

void DpaTransfer::SetStatus(DpaTransfer::DpaTransferStatus status)
{
  m_status = status;
}

bool DpaTransfer::IsInProgressStatus(DpaTransferStatus status)
{
  switch (status)
  {
  case kSent:
  case kConfirmation:
    return true;
  default:
    return false;
  }
}
