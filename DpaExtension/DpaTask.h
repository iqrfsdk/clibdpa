#pragma once

#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

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
  const std::string& getClid() const { return m_clid; }
  void setClid(const std::string& clid) { m_clid = clid; }
  int getAddress() const { return m_address; }
  void setAddress(int address) { m_address = address; }
  int getTimeout() const { return m_timeout; }
  void setTimeout(int timeout) { m_timeout = timeout; }
  int getCommand() const { return m_command; }
  bool isValid() const { return m_valid; }

protected:
  std::string m_prfName;
  std::string m_clid; //client ID
  bool m_valid = false;
  int m_address = -1;
  int m_timeout = -1;
  int m_command = -1;
  DpaMessage m_request;
};
