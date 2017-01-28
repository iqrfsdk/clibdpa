#include "PrfLeds.h"
#include "IqrfLogging.h"

const std::string  PrfLedG::PRF_NAME("LedG");
const std::string  PrfLedR::PRF_NAME("LedR");

const std::string STR_CMD_LED_SET_OFF("OFF");
const std::string STR_CMD_LED_SET_ON("ON");
const std::string STR_CMD_LED_GET("GET");
const std::string STR_CMD_LED_PULSE("PULSE");

PrfLed::PrfLed(const std::string& prfName, uint8_t colour)
  : DpaTask(prfName, colour)
{
  m_request.DpaPacket().DpaRequestPacket_t.PNUM = colour;
}

PrfLed::PrfLed(const std::string& prfName, uint8_t colour, uint16_t address, Cmd command)
  : DpaTask(prfName, colour, address, (uint8_t)command)
{
  m_request.DpaPacket().DpaRequestPacket_t.PNUM = colour;
}

PrfLed::~PrfLed()
{
}

uint8_t PrfLed::getColour() const
{
  return m_request.DpaPacket().DpaRequestPacket_t.PNUM;
}

void PrfLed::parseResponse(const DpaMessage& response)
{
  if (getPcmd() == (uint8_t)Cmd::GET) {
    m_ledState = response.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData[0];
  }
  else {
    m_ledState = -1; 
  }
}

PrfLed::Cmd PrfLed::getCmd() const
{
  return m_cmd;
}

void PrfLed::setCmd(PrfLed::Cmd cmd)
{
  m_cmd = cmd;
  setPcmd((uint8_t)m_cmd);
}

void PrfLed::parseCommand(const std::string& command)
{
  if (STR_CMD_LED_SET_OFF == command)
    setCmd(Cmd::SET_OFF);
  else if (STR_CMD_LED_SET_ON == command)
    setCmd(Cmd::SET_ON);
  else if (STR_CMD_LED_GET == command)
    setCmd(Cmd::GET);
  else if (STR_CMD_LED_PULSE == command)
    setCmd(Cmd::PULSE);
  else
    THROW_EX(std::logic_error, "Invalid command: " << PAR(command));
}

const std::string& PrfLed::encodeCommand() const
{
  switch (getCmd()) {
  case Cmd::SET_OFF:
    return STR_CMD_LED_SET_OFF;
  case Cmd::SET_ON:
    return STR_CMD_LED_SET_ON;
  case Cmd::GET:
    return STR_CMD_LED_GET;
  case Cmd::PULSE:
    return STR_CMD_LED_PULSE;
  default:
    THROW_EX(std::logic_error, "Invalid command: " << NAME_PAR(command, (uint8_t)getCmd()));
  }
}
