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

#pragma once

#include "DpaTask.h"
#include "DpaMessage.h"
#include <string>
#include <sstream>
#include <memory>

class PrfRaw : public DpaTask
{
public:
  static const std::string PRF_NAME;

  PrfRaw();
  PrfRaw(const DpaMessage& request);
  virtual ~PrfRaw();

  //from IQRF
  void parseResponse(const DpaMessage& response) override;

  //from Messaging
  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  void setRequest(const DpaMessage& request);

protected:
  DpaMessage m_confirmation;
  DpaMessage m_response;
};
