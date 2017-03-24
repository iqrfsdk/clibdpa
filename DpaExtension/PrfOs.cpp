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

#include "PrfOs.h"
#include "IqrfLogging.h"

const std::string PrfOs::PRF_NAME("OS");

// Commands
const std::string STR_CMD_READ("READ");
const std::string STR_CMD_RESET("RESET");
const std::string STR_CMD_READ_CFG("READ_CFG");
const std::string STR_CMD_RFPGM("RFPGM");
const std::string STR_CMD_SLEEP("SLEEP");
const std::string STR_CMD_BATCH("BATCH");
const std::string STR_CMD_SET_USEC("SET_USEC");
const std::string STR_CMD_SET_MID("SET_MID");
const std::string STR_CMD_RESTART("RESTART");
const std::string STR_CMD_WRITE_CFG_BYTE("WRITE_CFG_BYTE");
const std::string STR_CMD_LOAD_CODE("LOAD_CODE");
const std::string STR_CMD_WRITE_CFG("WRITE_CFG");

using namespace std::chrono;

PrfOs::PrfOs()
  :DpaTask(PRF_NAME, PNUM_OS)
{
}

PrfOs::PrfOs(uint16_t address)
  : DpaTask(PRF_NAME, PNUM_OS, address, (uint8_t)Cmd::READ)
{
}

PrfOs::~PrfOs()
{
}

void PrfOs::parseResponse(const DpaMessage& response)
{
  //TODO
  //m_8Temperature = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
}

void PrfOs::sleep(const std::chrono::seconds& sec, uint8_t ctrl)
{
  setCmd(Cmd::SLEEP);
  ctrl &= 0x0F; //reset milis flag
  milis2097 ms2097 = duration_cast<milis2097>(sec);
  uint16_t tm = (uint16_t)ms2097.count();
  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerOSSleep_Request.Time = tm;
  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerOSSleep_Request.Control = ctrl;
  m_request.SetLength(sizeof(TDpaIFaceHeader) + 3);
}

void PrfOs::sleep(const std::chrono::milliseconds& milis, uint8_t ctrl)
{
  setCmd(Cmd::SLEEP);
  ctrl &= 0x0F; //reset other flags
  ctrl |= 0x10; //set milis flags
  micros32768 mc32768 = duration_cast<micros32768>(milis);
  uint16_t tm = (uint16_t)mc32768.count();
  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerOSSleep_Request.Time = tm;
  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerOSSleep_Request.Control = ctrl;
  m_request.SetLength(sizeof(TDpaIFaceHeader) + 3);
}

void PrfOs::calibration()
{
  setCmd(Cmd::SLEEP);
  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerOSSleep_Request.Time = 0;
  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerOSSleep_Request.Control = (uint8_t)TimeControl::RUN_CALIB;
}

PrfOs::Cmd PrfOs::getCmd() const
{
  return m_cmd;
}

void PrfOs::setCmd(PrfOs::Cmd cmd)
{
  m_cmd = cmd;
  setPcmd((uint8_t)m_cmd);
}

void PrfOs::parseCommand(const std::string& command)
{
  if (STR_CMD_READ == command)
    setCmd(Cmd::READ);
  else if (STR_CMD_RESET == command)
    setCmd(Cmd::RESET);
  else if (STR_CMD_READ_CFG == command)
    setCmd(Cmd::READ_CFG);
  else if (STR_CMD_RFPGM == command)
    setCmd(Cmd::RFPGM);
  else if (STR_CMD_SLEEP == command)
    setCmd(Cmd::SLEEP);
  else if (STR_CMD_BATCH == command)
    setCmd(Cmd::BATCH);
  else if (STR_CMD_SET_USEC == command)
    setCmd(Cmd::SET_USEC);
  else if (STR_CMD_SET_MID == command)
    setCmd(Cmd::SET_MID);
  else if (STR_CMD_RESTART == command)
    setCmd(Cmd::RESTART);
  else if (STR_CMD_WRITE_CFG_BYTE == command)
    setCmd(Cmd::WRITE_CFG_BYTE);
  else if (STR_CMD_LOAD_CODE == command)
    setCmd(Cmd::LOAD_CODE);
  else if (STR_CMD_WRITE_CFG == command)
    setCmd(Cmd::WRITE_CFG);
  else
    THROW_EX(std::logic_error, "Invalid command: " << PAR(command));
}

const std::string& PrfOs::encodeCommand() const
{
  switch (getCmd()) {
  case Cmd::READ: return STR_CMD_READ;
  case Cmd::RESET: return STR_CMD_RESET;
  case Cmd::RFPGM: return STR_CMD_RFPGM;
  case Cmd::SLEEP: return STR_CMD_SLEEP;
  case Cmd::BATCH: return STR_CMD_BATCH;
  case Cmd::SET_USEC: return STR_CMD_SET_USEC;
  case Cmd::SET_MID: return STR_CMD_SET_MID;
  case Cmd::RESTART: return STR_CMD_RESTART;
  case Cmd::WRITE_CFG_BYTE: return STR_CMD_WRITE_CFG_BYTE;
  case Cmd::LOAD_CODE: return STR_CMD_LOAD_CODE;
  case Cmd::WRITE_CFG: return STR_CMD_WRITE_CFG;

  default:
    THROW_EX(std::logic_error, "Invalid command: " << NAME_PAR(command, (int)getCmd()));
  }
}
