#pragma once

#include "DpaMessage.h"

class DpaTask
{
public:
  DpaTask()
    :m_address(0)
  {}
  DpaTask(unsigned short address)
    :m_address(address)
  {}
  virtual ~DpaTask(){};
  virtual void parseResponse(const DpaMessage& response) = 0;
  virtual const DpaMessage& getRequest() const { return m_request; }
  unsigned short getAddress() { return m_address; }
protected:
  unsigned short m_address;
  DpaMessage m_request;
};

class DpaRawTask : public DpaTask
{
public:
  DpaRawTask(const DpaMessage& request);
  virtual ~DpaRawTask();
  virtual void parseResponse(const DpaMessage& response);
  const DpaMessage& getResponse() const { return m_response; }
private:
  DpaMessage m_response;
};

class DpaThermometer : public DpaTask
{
public:
  DpaThermometer(unsigned short address);
  virtual ~DpaThermometer();
  virtual void parseResponse(const DpaMessage& response);
  unsigned short getTemperature() { return m_temperature; }
private:
  unsigned short m_temperature;
};

