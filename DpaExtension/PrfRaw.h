#pragma once

#include "DpaTask.h"
#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

static const std::string  PRF_NAME_RawTask("RawTask");

class PrfRaw : public DpaTask
{
public:
  static const std::string PRF_NAME;

  PrfRaw();
  PrfRaw(const DpaMessage& request);
  virtual ~PrfRaw();

  //from IQRF 
  void parseConfirmation(const DpaMessage& confirmation) override;
  void parseResponse(const DpaMessage& response) override;

  //from Messaging 
  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  void setRequest(const DpaMessage& request);

protected:
  DpaMessage m_confirmation;
  DpaMessage m_response;
};
