#include "PrfLeds.h"
#include "IqrfLogging.h"

PrfLed::PrfLed(unsigned short address, int command, const std::string& prfname, int colour)
  :DpaTask(address, prfname, command)
  ,m_ledState(-1)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = (uint8_t)colour;
  packet.DpaRequestPacket_t.PCMD = (uint8_t)m_command;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

void PrfLed::parseResponse(const DpaMessage& response)
{
  if (m_command == GET) {
    m_ledState = response.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData[0];
  }
  else {
    m_ledState = -1; 
  }
}

void PrfLed::toStream(std::ostream& os) const
{
  os << PRF_NAME_LedG << "[" << getAddress() << "] ";
}

int PrfLed::convertCommand(const std::string& command)
{
  if ("CMD_LED_SET_OFF" == command)
    return CMD_LED_SET_OFF;
  else if ("CMD_LED_SET_ON" == command)
    return CMD_LED_SET_ON;
  else if ("CMD_LED_GET" == command)
    return CMD_LED_GET;
  else if ("CMD_LED_PULSE" == command)
    return CMD_LED_PULSE;
  else
    return CMD_LED_SET_OFF;
}
