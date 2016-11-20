#include "DpaTask.h"
#include "IqrfLogging.h"

//////////////////////////////
// class DpaRawTask
//////////////////////////////

DpaRawTask::DpaRawTask(const DpaMessage& request)
  :DpaTask(0, PRF_NAME_RawTask, 0)
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
  os << PRF_NAME_RawTask << "[" << getAddress() << "] " << FORM_HEX(m_request.DpaPacket().Buffer, m_request.Length());
}
