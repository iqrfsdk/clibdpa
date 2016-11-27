#include "PrfThermometer.h"
#include "IqrfLogging.h"

const std::string PrfThermometer::PRF_NAME("Thermometer");

const std::string STR_CMD_THERMOMETER_READ("READ");
const std::string STR_CMD_UNKNOWN("UNKNOWN");

PrfThermometer::PrfThermometer()
  :DpaTask(PRF_NAME)
{
}

PrfThermometer::PrfThermometer(int address, Cmd command)
  : DpaTask(PRF_NAME, address, (int)command)
{
}

PrfThermometer::~PrfThermometer()
{
}

const DpaMessage& PrfThermometer::getRequest()
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = m_address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;
  packet.DpaRequestPacket_t.PCMD = (uint8_t)m_command;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));

  return m_request;
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

void PrfThermometer::parseCommand(const std::string& command)
{
  m_valid = true;
  if (STR_CMD_THERMOMETER_READ == command)
    m_command =  CMD_THERMOMETER_READ;
  else
    m_valid = false;
}

const std::string& PrfThermometer::encodeCommand() const
{
  switch (m_command) {
  case CMD_THERMOMETER_READ:
    return STR_CMD_THERMOMETER_READ;
  default:
    return STR_CMD_UNKNOWN;
  }
}
