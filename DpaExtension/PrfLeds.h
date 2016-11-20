#pragma once

#include "DpaTask.h"

static const std::string  PRF_NAME_LedG("LedG");
static const std::string  PRF_NAME_LedR("LedR");

class PrfLed : public DpaTask
{
public:
  enum Cmd {
    SET_OFF = CMD_LED_SET_OFF,
    SET_ON = CMD_LED_SET_ON,
    GET = CMD_LED_GET,
    PULSE = CMD_LED_PULSE
  };

  PrfLed(unsigned short address, int command, const std::string& prfname, int colour);
  virtual ~PrfLed()
  {}
  virtual void parseResponse(const DpaMessage& response);
  virtual void toStream(std::ostream& os) const;
  int getLedState() const; // -1 uknown, 0 off, 1 on
  static int convertCommand(const std::string& command);
private:
  int m_ledState;
};

class PrfLedG : public PrfLed
{
public:
  PrfLedG(unsigned short address, int command)
    :PrfLed(address, command, PRF_NAME_LedG, PNUM_LEDG)
  {}
  virtual ~PrfLedG() {}
};

class PrfLedR : public PrfLed
{
public:
  PrfLedR(unsigned short address, int command)
    :PrfLed(address, command, PRF_NAME_LedR, PNUM_LEDR)
  {}
  virtual ~PrfLedR() {}
};
