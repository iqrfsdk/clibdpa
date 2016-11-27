#pragma once

#include "DpaTask.h"

class PrfLed : public DpaTask
{
public:
  enum class Cmd {
    SET_OFF = CMD_LED_SET_OFF,
    SET_ON = CMD_LED_SET_ON,
    GET = CMD_LED_GET,
    PULSE = CMD_LED_PULSE
  };

  PrfLed() = delete;
  PrfLed(const std::string& prfName, int colour);
  PrfLed(const std::string& prfName, int colour, int address, Cmd command);
  virtual ~PrfLed();
  
  const DpaMessage& getRequest() override;
  void parseResponse(const DpaMessage& response) override;

  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  int getLedState() const { return m_ledState; } // -1 uknown, 0 off, 1 on
  int getColour() const { return m_colour; }

protected:
  int m_ledState = -1;
  int m_colour = -1;
};

class PrfLedG : public PrfLed
{
public:
  static const std::string  PRF_NAME;

  PrfLedG()
    :PrfLed(PRF_NAME, PNUM_LEDG)
  {}

  PrfLedG(int address, Cmd command)
    :PrfLed(PRF_NAME, PNUM_LEDG, address, command)
  {}
  
  virtual ~PrfLedG()
  {}
};

class PrfLedR : public PrfLed
{
public:
  static const std::string  PRF_NAME;

  PrfLedR()
    :PrfLed(PRF_NAME, PNUM_LEDR)
  {}

  PrfLedR(int address, Cmd command)
    :PrfLed(PRF_NAME, PNUM_LEDR, address, command)
  {}

  virtual ~PrfLedR()
  {}
};
