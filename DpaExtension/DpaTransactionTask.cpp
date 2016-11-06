#include "DpaTransactionTask.h"

DpaTransactionTask::DpaTransactionTask(DpaTask& dpaTask)
  :m_dpaTask(dpaTask)
  , m_success(false)
{
}

DpaTransactionTask::~DpaTransactionTask()
{
}

void DpaTransactionTask::processConfirmationMessage(const DpaMessage& confirmation)
{
}

void DpaTransactionTask::processResponseMessage(const DpaMessage& response)
{
  m_dpaTask.parseResponse(response);
}

void DpaTransactionTask::processFinish(DpaRequest::DpaRequestStatus status)
{
  m_success = isProcessed(status);
}
