#include "DpaTransactionTask.h"
#include <chrono>

DpaTransactionTask::DpaTransactionTask(DpaTask& dpaTask)
  :m_dpaTask(dpaTask)
{
  m_future = m_promise.get_future();
}

DpaTransactionTask::~DpaTransactionTask()
{
}

const DpaMessage& DpaTransactionTask::getMessage() const
{
  return m_dpaTask.getRequest();
}

void DpaTransactionTask::processConfirmationMessage(const DpaMessage& confirmation)
{
  m_dpaTask.parseResponse(confirmation);
}

void DpaTransactionTask::processResponseMessage(const DpaMessage& response)
{
  m_dpaTask.parseResponse(response);
}

void DpaTransactionTask::processFinish(DpaRequest::DpaRequestStatus status)
{
  m_promise.set_value(isProcessed(status));
}

bool DpaTransactionTask::waitFinish(unsigned millis)
{
  std::chrono::milliseconds span(millis);
  if (m_future.wait_for(span) == std::future_status::timeout)
    return false;
  return m_future.get();
}
