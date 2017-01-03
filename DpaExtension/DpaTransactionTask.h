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
  
  const DpaMessage& getMessage() const override;
  int getTimeout() const override;
  void processConfirmationMessage(const DpaMessage& confirmation) override;
  void processResponseMessage(const DpaMessage& response) override;
  void processFinish(DpaRequest::DpaRequestStatus status) override;
  
  //0: success, -1: DpaHandler timeout, -2: future timeout, <n>: responseCode
  int waitFinish(unsigned millis = 8000);

  int getError() const;
  std::string getErrorStr() const;

private:
  DpaTask& m_dpaTask;
  std::promise<bool> m_promise;
  std::future<bool> m_future;
  int m_error;

};
