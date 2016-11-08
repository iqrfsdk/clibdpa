#include "DpaTask.h"
#include "IqrfLogging.h"

//////////////////////////////
// class DpaRawTask
//////////////////////////////

DpaRawTask::DpaRawTask(const DpaMessage& request)
  :DpaTask(0, NAME_RawTask)
{
  m_address = request.DpaPacket().DpaRequestPacket_t.NADR;
  m_request = request;
}

void DpaRawTask::parseConfirmation(const DpaMessage& confirmation)
{
  m_confirmation = confirmation;
}

void DpaRawTask::parseResponse(const DpaMessage& response)
{
  m_response = response;
}

void DpaRawTask::toStream(std::ostream& os) const
{
  os << NAME_RawTask << "[" << getAddress() << "] " << FORM_HEX(m_request.DpaPacket().Buffer, m_request.Length());
}

//////////////////////////////
// class DpaThermometer
//////////////////////////////

DpaThermometer::DpaThermometer(unsigned short address)
  :DpaTask(address, NAME_Thermometer)
  ,m_temperature(0)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;

  packet.DpaRequestPacket_t.PCMD = CMD_THERMOMETER_READ;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

void DpaThermometer::parseResponse(const DpaMessage& response)
{
  m_temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
}

void DpaThermometer::toStream(std::ostream& os) const
{
  os << NAME_Thermometer << "[" << getAddress() << "] " << getTemperature();
}

//////////////////////////////
// class DpaPulseLedG
//////////////////////////////

DpaPulseLedG::DpaPulseLedG(unsigned short address)
  :DpaTask(address, NAME_PulseLedG)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_LEDG;
  packet.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

void DpaPulseLedG::toStream(std::ostream& os) const
{
  os << NAME_PulseLedG << "[" << getAddress() << "] ";
}

//////////////////////////////
// class DpaPulseLedR
//////////////////////////////

DpaPulseLedR::DpaPulseLedR(unsigned short address)
  :DpaTask(address, NAME_PulseLedR)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_LEDR;
  packet.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

void DpaPulseLedR::toStream(std::ostream& os) const
{
  os << NAME_PulseLedR << "[" << getAddress() << "] ";
}
