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

#include "PrfOs.h"
#include "IqrfLogging.h"

const std::string PrfOs::PRF_NAME("std-per-os");

// Commands
const std::string STR_CMD_READ("read");
const std::string STR_CMD_RESET("reset");
const std::string STR_CMD_READ_CFG("read_cfg");
const std::string STR_CMD_RFPGM("rfpgm");
const std::string STR_CMD_SLEEP("sleep");
const std::string STR_CMD_BATCH("batch");
const std::string STR_CMD_SET_SECURITY("set_security");
const std::string STR_CMD_RESTART("restart");
const std::string STR_CMD_WRITE_CFG_BYTE("write_cfg_byte");
const std::string STR_CMD_LOAD_CODE("load_code");
const std::string STR_CMD_WRITE_CFG("write_cfg");

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
  switch (getCmd()) {

  case Cmd::READ: {
    TPerOSRead_Response resp = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerOSRead_Response;
    {
      std::ostringstream os;
      os.fill('0');

      os << std::hex <<
        std::setw(2) << (int)resp.ModuleId[3] <<
        std::setw(2) << (int)resp.ModuleId[2] <<
        std::setw(2) << (int)resp.ModuleId[1] <<
        std::setw(2) << (int)resp.ModuleId[0];
      m_moduleId = os.str();
    }

    {
      std::ostringstream os;
      os << std::hex <<
        std::setw(2) << (int)(resp.OsVersion >> 4) << '.';
      os.fill('0');
      os << std::setw(2) << (int)(resp.OsVersion & 0xf) << 'D';
      m_osVersion = os.str();
    }

    m_trType = (resp.ModuleId[3] & 0x80) ? "DCTR-" : "TR-";

    switch (resp.McuType >> 4) {
    case 0: m_trType += "52D"; break;
    case 1: m_trType += "58D-RJ"; break;
    case 2: m_trType += "72D"; break;
    case 3: m_trType += "53D"; break;
    case 8: m_trType += "54D"; break;
    case 9: m_trType += "55D"; break;
    case 10: m_trType += "56D"; break;
    case 11: m_trType += "76D"; break;
    default: m_trType += "???";
    }

    m_fcc = resp.McuType & 0x8;
    int pic = resp.McuType & 0x7;
    switch (pic) {
    case 3: m_mcuType = "PIC16F886"; break;
    case 4: m_mcuType = "PIC16F1938"; break;
    default: m_mcuType = "UNKNOWN";
    }

    {
      std::ostringstream os;
      os.fill('0');
      os << std::hex << std::setw(4) << (int)resp.OsBuild;
      m_osBuild = os.str();
    }
  }
                  break;

  default:;
  }
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

void PrfOs::read()
{
  setCmd(Cmd::READ);
  m_request.SetLength(sizeof(TDpaIFaceHeader));
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
  else if (STR_CMD_SET_SECURITY == command)
    setCmd(Cmd::SET_SECURITY);
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
  case Cmd::SET_SECURITY: return STR_CMD_SET_SECURITY;
  case Cmd::RESTART: return STR_CMD_RESTART;
  case Cmd::WRITE_CFG_BYTE: return STR_CMD_WRITE_CFG_BYTE;
  case Cmd::LOAD_CODE: return STR_CMD_LOAD_CODE;
  case Cmd::WRITE_CFG: return STR_CMD_WRITE_CFG;

  default:
    THROW_EX(std::logic_error, "Invalid command: " << NAME_PAR(command, (int)getCmd()));
  }
}
