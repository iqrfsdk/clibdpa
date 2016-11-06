#pragma once

class DpaMessage;

class DpaTransaction
{
public:
  virtual ~DpaTransaction() {}
  virtual const DpaMessage& getMessage() const = 0;
  virtual void processConfirmationMessage(const DpaMessage& confirmation) = 0;
  virtual void processResponseMessage(const DpaMessage& response) = 0;
  virtual void processFinished(bool success) = 0;
};
