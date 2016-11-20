#pragma once

#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

static const std::string  PRF_NAME_RawTask("RawTask");

class DpaTask
{
public:
  DpaTask(unsigned short address, const std::string& taskName, int command)
    :m_address(address)
    ,m_taskName(taskName)
    ,m_command(command)
  {}

  DpaTask() = delete;

  virtual ~DpaTask() {};
  virtual void parseConfirmation(const DpaMessage& confirmation) {}
  virtual void parseResponse(const DpaMessage& response) = 0;
  virtual const DpaMessage& getRequest() const { return m_request; }
  virtual void toStream(std::ostream& os) const = 0;
  const std::string& getTaskName() const { return m_taskName; }
  unsigned short getAddress() const { return m_address; }
  friend std::ostream& operator<<(std::ostream& o, const DpaTask& dt) {
    dt.toStream(o);
    return o;
  }
protected:
  unsigned short m_address;
  std::string m_taskName;
  int m_command;
  DpaMessage m_request;
};

class DpaRawTask : public DpaTask
{
public:
  DpaRawTask(const DpaMessage& request);
  virtual ~DpaRawTask() {}
  virtual void parseConfirmation(const DpaMessage& confirmation);
  virtual void parseResponse(const DpaMessage& response);
  const DpaMessage& getResponse() const { return m_response; }
  virtual void toStream(std::ostream& os) const;
private:
  DpaMessage m_confirmation;
  DpaMessage m_response;
};
