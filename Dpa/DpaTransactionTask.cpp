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

#include <chrono>

#include "DpaTransactionTask.h"

DpaTransactionTask::DpaTransactionTask(DpaTask& dpaTask) : m_dpaTask(dpaTask), m_error(0) {
  m_future = m_promise.get_future();
}

DpaTransactionTask::~DpaTransactionTask() {}

const DpaMessage& DpaTransactionTask::getMessage() const {
  m_dpaTask.timestampRequest();
  return m_dpaTask.getRequest();
}

int DpaTransactionTask::getTimeout() const {
  return m_dpaTask.getTimeout();
}

void DpaTransactionTask::processConfirmationMessage(const DpaMessage& confirmation) {
  m_dpaTask.handleConfirmation(confirmation);
}

void DpaTransactionTask::processResponseMessage(const DpaMessage& response) {
  m_error = response.DpaPacket().DpaResponsePacket_t.ResponseCode;
  m_dpaTask.handleResponse(response);
}

void DpaTransactionTask::processFinish(DpaTransfer::DpaTransferStatus status)
{
  switch (status) {
  case DpaTransfer::DpaTransferStatus::kError:
    m_error = -4;
    break;
  case DpaTransfer::DpaTransferStatus::kAborted:
    m_error = -3;
    break;
  case DpaTransfer::DpaTransferStatus::kTimeout:
    m_error = -1;
    break;
  default:;
  }
  m_promise.set_value(m_error);
}

int DpaTransactionTask::waitFinish()
{
  int timeout = m_dpaTask.getTimeout();
  if (timeout < 0) {
    // blocks until the result becomes available
    m_future.wait();
    m_error = m_future.get();
  }
  else {
    std::chrono::milliseconds span(m_dpaTask.getTimeout() * 2);
    if (m_future.wait_for(span) == std::future_status::timeout) {
      m_error = -2;
    }
    else {
      m_error = m_future.get();
    }
  }
  return m_error;
}

int DpaTransactionTask::getError() const
{
  return m_error;
}

std::string DpaTransactionTask::getErrorStr() const
{
  switch (m_error) {
  case -4:
    return "ERROR_IFACE";
  case -3:
    return "ERROR_ABORTED";
  case -2:
    return "ERROR_PROMISE_TIMEOUT";
  case -1:
    return "ERROR_TIMEOUT";
  case STATUS_NO_ERROR:
    return "STATUS_NO_ERROR";
  case ERROR_FAIL:
    return "ERROR_FAIL";
  case ERROR_PCMD:
    return "ERROR_PCMD";
  case ERROR_PNUM:
    return "ERROR_PNUM";
  case ERROR_ADDR:
    return "ERROR_ADDR";
  case ERROR_DATA_LEN:
    return "ERROR_DATA_LEN";
  case ERROR_DATA:
    return "ERROR_DATA";
  case ERROR_HWPID:
    return "ERROR_HWPID";
  case ERROR_NADR:
    return "ERROR_NADR";
  case ERROR_IFACE_CUSTOM_HANDLER:
    return "ERROR_IFACE_CUSTOM_HANDLER";
  case ERROR_MISSING_CUSTOM_DPA_HANDLER:
    return "ERROR_MISSING_CUSTOM_DPA_HANDLER";
  case ERROR_USER_TO:
    return "ERROR_USER_TO";
  case STATUS_CONFIRMATION:
    return "STATUS_CONFIRMATION";
  case ERROR_USER_FROM:
  default:
    std::ostringstream os;
    os << std::hex << m_error;
    return os.str();
  }
}
