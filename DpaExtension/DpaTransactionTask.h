#pragma once

#include "DpaTransaction.h"
#include "DpaTask.h"

class DpaTransactionTask : public DpaTransaction
{
public:
  DpaTransactionTask(DpaTask& dpaTask);
  virtual ~DpaTransactionTask();
  virtual const DpaMessage& getMessage() const { return m_dpaTask.getRequest(); }
  virtual void processConfirmationMessage(const DpaMessage& confirmation);
  virtual void processResponseMessage(const DpaMessage& response);
  virtual void processFinish(DpaRequest::DpaRequestStatus status);
  bool isSuccess() { return m_success; }
private:
  DpaTask& m_dpaTask;
  bool m_success;
};
