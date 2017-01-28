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
  PrfLed(const std::string& prfName, uint8_t colour);
  PrfLed(const std::string& prfName, uint8_t colour, uint16_t address, Cmd command);
  virtual ~PrfLed();
  
  void parseResponse(const DpaMessage& response) override;

  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  int getLedState() const { return m_ledState; } // -1 uknown, 0 off, 1 on
  uint8_t getColour() const;

  Cmd getCmd() const;
  void setCmd(Cmd cmd);

protected:
  Cmd m_cmd = Cmd::SET_OFF;
  int m_ledState = -1;
};

class PrfLedG : public PrfLed
{
public:
  static const std::string  PRF_NAME;

  PrfLedG()
    :PrfLed(PRF_NAME, PNUM_LEDG)
  {}

  PrfLedG(uint16_t address, Cmd command)
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

  PrfLedR(uint16_t address, Cmd command)
    :PrfLed(PRF_NAME, PNUM_LEDR, address, command)
  {}

  virtual ~PrfLedR()
  {}
};
