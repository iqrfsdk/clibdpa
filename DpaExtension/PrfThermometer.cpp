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

#include "PrfThermometer.h"
#include "IqrfLogging.h"

const std::string PrfThermometer::PRF_NAME("Thermometer");

const std::string STR_CMD_THERMOMETER_READ("READ");

PrfThermometer::PrfThermometer()
  :DpaTask(PRF_NAME, PNUM_THERMOMETER)
{
}

PrfThermometer::PrfThermometer(uint16_t address, Cmd command)
  : DpaTask(PRF_NAME, PNUM_THERMOMETER, address, (uint8_t)command)
{
}

PrfThermometer::~PrfThermometer()
{
}

void PrfThermometer::parseResponse(const DpaMessage& response)
{
  m_8Temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
  m_16Temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.SixteenthValue;

  if (m_8Temperature & 0x80) { //negative value //TODO verify where is SIGN
    m_intTemperature = m_8Temperature & 0x7F;
    m_intTemperature = -m_intTemperature;
  }
  else
    m_intTemperature = m_8Temperature;

  int tempi;
  if (m_16Temperature & 0x8000) { //negative value //TODO verify where is SIGN
    tempi = m_16Temperature & 0x7FFF;
    tempi = -tempi;
  }
  else
    tempi = m_16Temperature;
  m_floatTemperature = tempi * 0.0625; // *1/16
}

PrfThermometer::Cmd PrfThermometer::getCmd() const
{
  return m_cmd;
}

void PrfThermometer::setCmd(PrfThermometer::Cmd cmd)
{
  m_cmd = cmd;
  setPcmd((uint8_t)m_cmd);
}

void PrfThermometer::parseCommand(const std::string& command)
{
  if (STR_CMD_THERMOMETER_READ == command)
    setCmd(Cmd::READ);
  else
    THROW_EX(std::logic_error, "Invalid command: " << PAR(command));
}

const std::string& PrfThermometer::encodeCommand() const
{
  switch (getCmd()) {
  case Cmd::READ:
    return STR_CMD_THERMOMETER_READ;
  default:
    THROW_EX(std::logic_error, "Invalid command: " << NAME_PAR(command, (uint8_t)getCmd()));
  }
}
