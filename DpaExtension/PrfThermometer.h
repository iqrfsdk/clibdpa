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

class PrfThermometer : public DpaTask
{
public:
  enum class Cmd {
    READ = CMD_THERMOMETER_READ
  };

  static const std::string PRF_NAME;

  PrfThermometer();
  PrfThermometer(uint16_t address, Cmd command);
  virtual ~PrfThermometer();

  void parseResponse(const DpaMessage& response) override;
  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  int getIntTemperature() const { return m_intTemperature; }
  float getFloatTemperature() const { return m_floatTemperature; }
  uint8_t getRaw8Temperature() const { return m_8Temperature; }
  uint16_t getRaw16Temperature() const { return m_16Temperature; }

  Cmd getCmd() const;
  void setCmd(Cmd cmd);

private:
  Cmd m_cmd = Cmd::READ;
  int m_intTemperature = -273;
  float m_floatTemperature = -273.15f;
  uint8_t m_8Temperature = 0;
  uint16_t m_16Temperature = 0;
};
