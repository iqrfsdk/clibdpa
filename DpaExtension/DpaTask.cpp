#include "DpaTask.h"
#include "IqrfLogging.h"

const std::string DpaRawTask::PRF_NAME("Raw");

const std::string STR_CMD_UNKNOWN("UNKNOWN");

DpaRawTask::DpaRawTask()
  :DpaTask(DpaRawTask::PRF_NAME)
{
}

DpaRawTask::DpaRawTask(const DpaMessage& request)
  :DpaTask(PRF_NAME_RawTask)
{
  setRequest(request);
}

void DpaRawTask::setRequest(const DpaMessage& request)
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

void DpaRawTask::parseCommand(const std::string& command)
{
}

const std::string& DpaRawTask::encodeCommand() const
{
  return STR_CMD_UNKNOWN;
}

const DpaMessage& DpaRawTask::getRequest()
{
  return m_request;
}
