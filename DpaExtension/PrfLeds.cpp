#include "PrfLeds.h"
#include "IqrfLogging.h"

const std::string  PrfLedG::PRF_NAME("LedG");
const std::string  PrfLedR::PRF_NAME("LedR");

const std::string STR_CMD_LED_SET_OFF("OFF");
const std::string STR_CMD_LED_SET_ON("ON");
const std::string STR_CMD_LED_GET("GET");
const std::string STR_CMD_LED_PULSE("PULSE");
const std::string STR_CMD_UNKNOWN("UNKNOWN");

PrfLed::PrfLed(const std::string& prfName, int colour)
  : DpaTask(prfName)
{
  m_colour = colour;
}

PrfLed::PrfLed(const std::string& prfName, int colour, int address, Cmd command)
  : DpaTask(prfName, address, (int)command)
{
  m_colour = colour;
}

PrfLed::~PrfLed()
{
}

const DpaMessage& PrfLed::getRequest()
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = m_address;
  packet.DpaRequestPacket_t.PNUM = (uint8_t)m_colour;
  packet.DpaRequestPacket_t.PCMD = (uint8_t)m_command;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));

  return m_request;
}

void PrfLed::parseResponse(const DpaMessage& response)
{
  if (m_command == (int)Cmd::GET) {
    m_ledState = response.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData[0];
  }
  else {
    m_ledState = -1; 
  }
}

void PrfLed::parseCommand(const std::string& command)
{
  m_valid = true;
  if (STR_CMD_LED_SET_OFF == command)
    m_command = CMD_LED_SET_OFF;
  else if (STR_CMD_LED_SET_ON == command)
    m_command = CMD_LED_SET_ON;
  else if (STR_CMD_LED_GET == command)
    m_command = CMD_LED_GET;
  else if (STR_CMD_LED_PULSE == command)
    m_command = CMD_LED_PULSE;
  else
    m_valid = false;
}

const std::string& PrfLed::encodeCommand() const
{
  switch (m_command) {
  case CMD_LED_SET_OFF:
    return STR_CMD_LED_SET_OFF;
  case CMD_LED_SET_ON:
    return STR_CMD_LED_SET_ON;
  case CMD_LED_GET:
    return STR_CMD_LED_GET;
  case CMD_LED_PULSE:
    return STR_CMD_LED_PULSE;
  default:
    return STR_CMD_UNKNOWN;
  }
}
