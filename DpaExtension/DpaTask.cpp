#include "DpaTask.h"

//////////////////////////////
// class DpaRawTask
//////////////////////////////

DpaRawTask::DpaRawTask(const DpaMessage& request)
  :DpaTask()
{
  m_address = request.DpaPacket().DpaRequestPacket_t.NADR;
  m_request = request;
}

DpaRawTask::~DpaRawTask()
{
}

void DpaRawTask::parseResponse(const DpaMessage& response)
{
  m_response = response;
}

//////////////////////////////
// class DpaThermometer
//////////////////////////////

DpaThermometer::DpaThermometer(unsigned short address)
  :DpaTask(address)
  , m_temperature(0)
{
  DpaMessage::DpaPacket_t packet;
  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;

  packet.DpaRequestPacket_t.PCMD = CMD_THERMOMETER_READ;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));
}

DpaThermometer::~DpaThermometer()
{
}

void DpaThermometer::parseResponse(const DpaMessage& response)
{
  //TODO some checks
  m_temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
}

//////////////////////////////
// class DpaPulseLed
//////////////////////////////

DpaPulseLed::DpaPulseLed(unsigned short address, LedColor color)
  :m_color(color)
  ,DpaTask(address)
{
  DpaMessage::DpaPacket_t packet;
  packet.DpaRequestPacket_t.NADR = address;
  if (color == kLedRed)
    packet.DpaRequestPacket_t.PNUM = PNUM_LEDR;
  else
    packet.DpaRequestPacket_t.PNUM = PNUM_LEDG;

  packet.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));
}

DpaPulseLed::~DpaPulseLed()
{
}

void DpaPulseLed::parseResponse(const DpaMessage& response)
{
}
