#include "DpaTask.h"

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

  m_request.AddDataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));
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
