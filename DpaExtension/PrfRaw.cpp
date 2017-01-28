#include "PrfRaw.h"
#include "IqrfLogging.h"

const std::string PrfRaw::PRF_NAME("Raw");

const std::string STR_CMD_UNKNOWN("UNKNOWN");

PrfRaw::PrfRaw()
  :DpaTask(PrfRaw::PRF_NAME, 0)
{
}

PrfRaw::PrfRaw(const DpaMessage& request)
  :DpaTask(PRF_NAME_RawTask, 0)
{
  setRequest(request);
}

PrfRaw::~PrfRaw()
{
}

void PrfRaw::setRequest(const DpaMessage& request)
{
  m_request = request;
}

void PrfRaw::parseConfirmation(const DpaMessage& confirmation)
{
  m_confirmation = confirmation;
}

void PrfRaw::parseResponse(const DpaMessage& response)
{
  m_response = response;
}

void PrfRaw::parseCommand(const std::string& command)
{
}

const std::string& PrfRaw::encodeCommand() const
{
  return STR_CMD_UNKNOWN;
}
