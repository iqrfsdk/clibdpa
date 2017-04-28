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

#include "PrfRawHdp.h"
#include "IqrfLogging.h"

const std::string PrfRawHdp::PRF_NAME("raw-hdp");

const std::string STR_CMD_UNKNOWN("UNKNOWN");

PrfRawHdp::PrfRawHdp()
{
}

PrfRawHdp::PrfRawHdp(const DpaMessage& request)
  //:PrfRaw(PRF_NAME, 0)
{
  setRequest(request);
}

PrfRawHdp::~PrfRawHdp()
{
}

void PrfRawHdp::setRequest(const DpaMessage& request)
{
  m_request = request;
}

void PrfRawHdp::parseCommand(const std::string& command)
{
}

const std::string& PrfRawHdp::encodeCommand() const
{
  return STR_CMD_UNKNOWN;
}
