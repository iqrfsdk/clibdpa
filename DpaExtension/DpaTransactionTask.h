#pragma once

#include "DpaTransaction.h"
#include "DpaTask.h"
#include <future>

class DpaTransactionTask : public DpaTransaction
{
public:
  DpaTransactionTask(DpaTask& dpaTask);
  DpaTransactionTask() = delete;
  DpaTransactionTask(const DpaTransactionTask&) = delete;
  DpaTransactionTask(DpaTransactionTask&&) = delete;
  DpaTransactionTask& operator= (const DpaTransactionTask&) = delete;
  DpaTransactionTask& operator= (DpaTransactionTask&&) = delete;

  virtual ~DpaTransactionTask();
  
  virtual const DpaMessage& getMessage() const;
  virtual void processConfirmationMessage(const DpaMessage& confirmation);
  virtual void processResponseMessage(const DpaMessage& response);
  virtual void processFinish(DpaRequest::DpaRequestStatus status);
  
  bool waitFinish(unsigned millis = 8000);

private:
  DpaTask& m_dpaTask;
  std::promise<bool> m_promise;
  std::future<bool> m_future;

};
