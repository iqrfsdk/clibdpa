#pragma once

class DpaMessage;

class IDpaResponseHandler
{
 public:
   virtual ~IDpaResponseHandler() {}
   virtual void ProcessConfirmationMessage(const DpaMessage& confirmation_message) = 0;
   virtual void ProcessResponseMessage(const DpaMessage& response_message) = 0;
};
