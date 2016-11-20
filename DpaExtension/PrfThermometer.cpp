#include "PrfThermometer.h"
#include "IqrfLogging.h"

//////////////////////////////
// class PrfThermometer
//////////////////////////////

PrfThermometer::PrfThermometer(unsigned short address, int command)
  :DpaTask(address, PRF_NAME_Thermometer, command)
  ,m_intTemperature(-128)
  ,m_floatTemperature(-128.0)
  ,m_8Temperature(0)
  ,m_16Temperature(0)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;
  packet.DpaRequestPacket_t.PCMD = (uint8_t)m_command;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

void PrfThermometer::parseResponse(const DpaMessage& response)
{
  m_8Temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
  m_16Temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.SixteenthValue;

  if (m_8Temperature & 0x80) { //negative value //TODO verify where is SIGN
    m_intTemperature = m_8Temperature & 0x7F;
    m_intTemperature = -m_intTemperature;
  }
  else
    m_intTemperature = m_8Temperature;

  int tempi;
  if (m_16Temperature & 0x8000) { //negative value //TODO verify where is SIGN
    tempi = m_16Temperature & 0x7FFF;
    tempi = -tempi;
  }
  else
    tempi = m_16Temperature;
  m_floatTemperature = tempi * 0.0625; // *1/16
}

void PrfThermometer::toStream(std::ostream& os) const
{
  os << PRF_NAME_Thermometer << " " << getAddress() << " " << getIntTemperature() << " " << getFloatTemperature();
}

int PrfThermometer::convertCommand(const std::string& command)
{
  if ("CMD_THERMOMETER_READ" == command)
    return CMD_THERMOMETER_READ;
  else
    return CMD_THERMOMETER_READ;
}
