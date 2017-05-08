/**
 * Copyright 2015-2017 MICRORISC s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
