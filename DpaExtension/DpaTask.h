#pragma once

#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

static const std::string  NAME_RawTask("RawTask");
static const std::string  NAME_Thermometer("Thermometer");
static const std::string  NAME_PulseLedG("PulseLedG");
static const std::string  NAME_PulseLedR("PulseLedR");

class DpaTask
{
public:
  DpaTask(unsigned short address, const std::string& taskName)
    :m_address(address)
    , m_taskName(taskName)
  {}

  DpaTask() = delete;

  virtual ~DpaTask() {};
  virtual void parseConfirmation(const DpaMessage& confirmation) {}
  virtual void parseResponse(const DpaMessage& response) = 0;
  virtual const DpaMessage& getRequest() const { return m_request; }
  virtual void toStream(std::ostream& os) const = 0;
  unsigned short getAddress() const { return m_address; }
  friend std::ostream& operator<<(std::ostream& o, const DpaTask& dt) {
    dt.toStream(o);
    return o;
  }
protected:
  unsigned short m_address;
  std::string m_taskName;
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

class DpaThermometer : public DpaTask
{
public:
  DpaThermometer(unsigned short address);
  virtual ~DpaThermometer() {}
  virtual void parseResponse(const DpaMessage& response);
  unsigned short getTemperature() const { return m_temperature; }
  virtual void toStream(std::ostream& os) const;
private:
  unsigned short m_temperature;
};

class DpaPulseLedG : public DpaTask
{
public:
  DpaPulseLedG(unsigned short address);
  virtual ~DpaPulseLedG() {};
  virtual void parseResponse(const DpaMessage& response) {};
  virtual void toStream(std::ostream& os) const;
};

class DpaPulseLedR : public DpaTask
{
public:
  DpaPulseLedR(unsigned short address);
  virtual ~DpaPulseLedR() {};
  virtual void parseResponse(const DpaMessage& response) {};
  virtual void toStream(std::ostream& os) const;
};
