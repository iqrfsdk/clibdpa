#include "DpaTask.h"
#include "IqrfLogging.h"

DpaTask::DpaTask(const std::string& prfName, uint8_t prfNum)
  :m_prfName(prfName)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.PNUM = prfNum;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

DpaTask::DpaTask(const std::string& prfName, uint8_t prfNum, uint16_t address, uint8_t command)
  :m_prfName(prfName)
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = prfNum;
  packet.DpaRequestPacket_t.PCMD = command;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;
  
  m_request.SetLength(sizeof(TDpaIFaceHeader));
}

DpaTask::~DpaTask()
{}

uint16_t DpaTask::getAddress() const
{
  return m_request.DpaPacket().DpaRequestPacket_t.NADR;
}

void DpaTask::setAddress(uint16_t address)
{
  m_request.DpaPacket().DpaRequestPacket_t.NADR = address;
}

uint8_t DpaTask::getPcmd() const
{
  return m_request.DpaPacket().DpaRequestPacket_t.PCMD;
}

void DpaTask::setPcmd(uint8_t command)
{
  m_request.DpaPacket().DpaRequestPacket_t.PCMD = command;
}
