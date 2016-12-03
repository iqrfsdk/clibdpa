#pragma once

#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

static const std::string  PRF_NAME_RawTask("RawTask");

class DpaTask
{
public:
  DpaTask() = delete;

  DpaTask(const std::string& prfName)
    :m_prfName(prfName)
  {}

  DpaTask(const std::string& prfName, int address, int command)
    : m_prfName(prfName)
    , m_address(address)
    , m_command(command)
  {}

  virtual ~DpaTask()
  {}

  //from IQRF 
  virtual const DpaMessage& getRequest() = 0;
  virtual void parseConfirmation(const DpaMessage& confirmation) {}
  virtual void parseResponse(const DpaMessage& response) = 0;

  //from Messaging 
  virtual void parseCommand(const std::string& command) = 0;
  virtual const std::string& encodeCommand() const = 0;
  virtual std::string encodeResponse(const std::string& errStr) const { return std::string(); }

  const std::string& getPrfName() const { return m_prfName; }
  int getAddress() const { return m_address; }
  void setAddress(int address) { m_address = address; }
  int getCommand() const { return m_command; }
  bool isValid() const { return m_valid; }

protected:
  std::string m_prfName;
  bool m_valid = false;
  int m_address = -1;
  int m_command = -1;
  DpaMessage m_request;
};

class DpaRawTask : public DpaTask
{
public:
  static const std::string PRF_NAME;

  DpaRawTask();
  DpaRawTask(const DpaMessage& request);
  virtual ~DpaRawTask() {}

  //from IQRF 
  const DpaMessage& getRequest() override;
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
