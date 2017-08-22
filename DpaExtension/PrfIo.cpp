/**
 * Copyright 2015-2017 MICRORISC s.r.o.
 * Copyright 2017 IQRF Tech s.r.o.
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

#include "PrfIo.h"
#include "IqrfLogging.h"


const std::string PrfIo::PRF_NAME("std-per-io");

// Commands
const std::string STR_CMD_DIRECTION("direction");
const std::string STR_CMD_SET("set");
const std::string STR_CMD_GET("get");

// Port names
const std::string STR_PORTA("porta");
const std::string STR_PORTB("portb");
const std::string STR_PORTC("portc");
const std::string STR_PORTE("porte");
const std::string STR_WPUB("wpub");
const std::string STR_WPUE("wpue");
//DELAY = 0xff

using namespace std::chrono;

PrfIo::PrfIo()
  :DpaTask(PRF_NAME, PNUM_IO)
{
}

PrfIo::PrfIo(uint16_t address)
  : DpaTask(PRF_NAME, PNUM_IO, address, (uint8_t)Cmd::GET)
{
}

PrfIo::~PrfIo()
{
}

void PrfIo::parseResponse(const DpaMessage& response)
{
  const uint8_t* p = response.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData;
  if (getPcmd() == (uint8_t)Cmd::GET) {
    int sz = response.Length() - sizeof(TDpaIFaceHeader) - 2; // - (ResponseCode + DpaValue)
    int i = 0;
    while (true) {
      //TODO better IO support? Why it does not reply with [port,value]?
      m_PORTA = p[i];
      if (++i == sz) break;
      m_PORTB = p[i];
      if (++i == sz) break;
      m_PORTC = p[i];
      if (++i == sz) break;
      m_PORTE = p[i];
      if (++i == sz) break;
      m_WPUB = p[i];
      if (++i == sz) break;
      m_WPUE = p[i];
      if (++i == sz) break;
    }
  }
}

void PrfIo::directionCommand(DirectionAndSet directions)
{
  setCmd(Cmd::DIRECTION);

  size_t maxSize = DPA_MAX_DATA_LENGTH / sizeof(TPerIOTriplet);
  if (directions.size() > maxSize)
    THROW_EX(std::logic_error, PAR(maxSize) << NAME_PAR(dirSize, directions.size()) << " too long");

  maxSize = directions.size();
  TDpaMessage& dpaMsg = m_request.DpaPacket().DpaRequestPacket_t.DpaMessage;

  for (size_t i = 0; i < maxSize; i++) {
    auto& d = directions[i];
    auto& trip = dpaMsg.PerIoDirectionAndSet_Request.Triplets[i];
    trip.Port = (uint8_t)std::get<0>(d);
    trip.Mask = std::get<1>(d);
    trip.Value = std::get<2>(d);
  }
  m_request.SetLength(sizeof(TDpaIFaceHeader) + sizeof(TPerIOTriplet) * maxSize);
}

void PrfIo::directionCommand(Port port, uint8_t bit, bool direction)
{
  setCmd(Cmd::DIRECTION);

  if (bit > 7)
    THROW_EX(std::logic_error, PAR(bit) << " is invalid");

  TDpaMessage& dpaMsg = m_request.DpaPacket().DpaRequestPacket_t.DpaMessage;

  auto& trip = dpaMsg.PerIoDirectionAndSet_Request.Triplets[0];
  trip.Port = (uint8_t)port;
  trip.Mask = 1 << bit;
  //trip.Value = direction ? 0xff : 0;
  trip.Value = direction ? trip.Mask : 0;

  m_request.SetLength(sizeof(TDpaIFaceHeader) + sizeof(TPerIOTriplet));
}

void PrfIo::setCommand(DirectionAndSet settings)
{
  setCmd(Cmd::SET);

  size_t maxSize = DPA_MAX_DATA_LENGTH / sizeof(TPerIOTriplet);
  if (settings.size() > maxSize)
    THROW_EX(std::logic_error, PAR(maxSize) << NAME_PAR(dirSize, settings.size()) << " too long");

  maxSize = settings.size();
  TDpaMessage& dpaMsg = m_request.DpaPacket().DpaRequestPacket_t.DpaMessage;

  for (size_t i = 0; i < maxSize; i++) {
    auto& s = settings[i];
    auto& trip = dpaMsg.PerIoDirectionAndSet_Request.Triplets[i];
    trip.Port = (uint8_t)std::get<0>(s);
    trip.Mask = std::get<1>(s);
    trip.Value = std::get<2>(s);
  }
  m_request.SetLength(sizeof(TDpaIFaceHeader) + sizeof(TPerIOTriplet) * maxSize);
}

void PrfIo::setCommand(Port port, uint8_t bit, bool val)
{
  setCmd(Cmd::SET);

  if (bit > 7)
    THROW_EX(std::logic_error, PAR(bit) << " is invalid");

  TDpaMessage& dpaMsg = m_request.DpaPacket().DpaRequestPacket_t.DpaMessage;

  auto& trip = dpaMsg.PerIoDirectionAndSet_Request.Triplets[0];
  trip.Port = (uint8_t)port;
  trip.Mask = 1 << bit;
  //trip.Value = val ? 0xff : 0;
  trip.Value = val ? trip.Mask : 0;

  m_request.SetLength(sizeof(TDpaIFaceHeader) + sizeof(TPerIOTriplet));
}

void PrfIo::getCommand()
{
  setCmd(Cmd::GET);
}

void PrfIo::getCommand(Port port, uint8_t bit)
{
  setCmd(Cmd::GET);
}

void PrfIo::addDirection(DirectionAndSet& das, Port port, uint8_t bits, uint8_t dirs)
{
  std::tuple<Port, uint8_t, uint8_t> dir(port, bits, dirs);
  das.push_back(dir);
}

void PrfIo::addTriplet(DirectionAndSet& das, Port port, uint8_t bits, uint8_t vals)
{
  std::tuple<Port, uint8_t, uint8_t> triplet(port, bits, vals);
  das.push_back(triplet);
}

void PrfIo::addDelay(DirectionAndSet& das, uint16_t delay)
{
  std::tuple<Port, uint8_t, uint8_t> d(Port::DELAY, (uint8_t)(delay & 0xff), (uint8_t)(delay >> 8));
  das.push_back(d);
}

uint8_t PrfIo::getInput(Port port) const
{
  uint8_t val = 0;

  switch (port) {
  case Port::PORTA:
    return m_PORTA;
  case Port::PORTB:
    return m_PORTB;
  case Port::PORTC:
    return m_PORTC;
  case Port::PORTE:
    return m_PORTE;
  case Port::WPUB:
    return m_WPUB;
  case Port::WPUE:
    return m_WPUE;
  default:
    THROW_EX(std::logic_error, "Unexpected: " << NAME_PAR(port, (uint8_t)port));
  }
}

bool PrfIo::getInput(Port port, uint8_t bit) const
{
  if (bit > 7)
    THROW_EX(std::logic_error, "Unexpected: " << PAR(bit));

  uint8_t val = getInput(port);
  return (val & (1 << bit)) > 0;
}

PrfIo::Cmd PrfIo::getCmd() const
{
  return m_cmd;
}

void PrfIo::setCmd(PrfIo::Cmd cmd)
{
  m_cmd = cmd;
  setPcmd((uint8_t)m_cmd);
}

void PrfIo::parseCommand(const std::string& command)
{
  if (STR_CMD_DIRECTION == command)
    setCmd(Cmd::DIRECTION);
  else if (STR_CMD_SET == command)
    setCmd(Cmd::SET);
  else if (STR_CMD_GET == command)
    setCmd(Cmd::GET);
  else
    THROW_EX(std::logic_error, "Invalid command: " << PAR(command));
}

const std::string& PrfIo::encodeCommand() const
{
  switch (getCmd()) {
  case Cmd::DIRECTION: return STR_CMD_DIRECTION;
  case Cmd::SET: return STR_CMD_SET;
  case Cmd::GET: return STR_CMD_GET;

  default:
    THROW_EX(std::logic_error, "Invalid command: " << NAME_PAR(command, (int)getCmd()));
  }
}

PrfIo::Port PrfIo::parsePort(const std::string& port)
{
  if (STR_PORTA == port)
    return Port::PORTA;
  else if (STR_PORTB == port)
    return Port::PORTB;
  else if (STR_PORTC == port)
    return Port::PORTC;
  else if (STR_PORTE == port)
    return Port::PORTE;
  else if (STR_WPUB == port)
    return Port::WPUB;
  else if (STR_WPUE == port)
    return Port::WPUE;
  else
    THROW_EX(std::logic_error, "Invalid port: " << PAR(port));
}

const std::string& PrfIo::encodePort(Port port)
{
  switch (port) {
  case Port::PORTA: return STR_PORTA;
  case Port::PORTB: return STR_PORTB;
  case Port::PORTC: return STR_PORTC;
  case Port::PORTE: return STR_PORTE;
  case Port::WPUB: return STR_WPUB;
  case Port::WPUE: return STR_WPUE;

  default:
    THROW_EX(std::logic_error, "Invalid port: " << NAME_PAR(port, (int)port));
  }
}
