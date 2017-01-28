#pragma once

#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

class DpaTask
{
public:
  DpaTask() = delete;
  DpaTask(const std::string& prfName, uint8_t prfNum);
  DpaTask(const std::string& prfName, uint8_t prfNum, uint16_t address, uint8_t command);
  virtual ~DpaTask();

  //from IQRF 
  const DpaMessage& getRequest() const { return m_request; }
  virtual void parseConfirmation(const DpaMessage& confirmation) {}
  virtual void parseResponse(const DpaMessage& response) = 0;

  //from Messaging 
  virtual void parseCommand(const std::string& command) = 0;
  virtual const std::string& encodeCommand() const = 0;
  virtual std::string encodeResponse(const std::string& errStr) const { return std::string(); }

  const std::string& getPrfName() const { return m_prfName; }
  const std::string& getClid() const { return m_clid; }
  void setClid(const std::string& clid) { m_clid = clid; }
  uint16_t getAddress() const;
  void setAddress(uint16_t address);
  uint8_t getPcmd() const;
  void setPcmd(uint8_t command);
  int getTimeout() const { return m_timeout; }
  void setTimeout(int timeout) { m_timeout = timeout; }

protected:
  std::string m_prfName;
  std::string m_clid; //client ID
  int m_timeout = -1;
  DpaMessage m_request;
};
