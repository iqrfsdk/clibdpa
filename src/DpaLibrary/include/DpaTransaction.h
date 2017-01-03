#pragma once

#include "DpaRequest.h"
class DpaMessage;

class DpaTransaction
{
public:
  virtual ~DpaTransaction() {}
  virtual const DpaMessage& getMessage() const = 0;
  virtual int getTimeout() const = 0;
  virtual void processConfirmationMessage(const DpaMessage& confirmation) = 0;
  virtual void processResponseMessage(const DpaMessage& response) = 0;
  virtual void processFinish(DpaRequest::DpaRequestStatus status) = 0;
  bool isProcessed(DpaRequest::DpaRequestStatus status) { return status == DpaRequest::kProcessed ? true : false; }
};

