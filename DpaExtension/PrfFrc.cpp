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

#include "PrfFrc.h"
#include "IqrfLogging.h"

const std::string PrfFrc::PRF_NAME("std-per-frc");

// Commands
const std::string STR_CMD_SEND("send");
const std::string STR_CMD_EXTRARESULT("extraresult");
const std::string STR_CMD_SEND_SELECTIVE("send_selective");
const std::string STR_CMD_SET_PARAMS("set_params");

// Type of collected data
const std::string STR_GET_BIT2("get_bit2");
const std::string STR_GET_BYTE("get_byte");
const std::string STR_GET_BYTE2("get_byte2");

// Predefined FRC commands
const std::string STR_FRC_Prebonding("prebonding");
const std::string STR_FRC_UART_SPI_data("uart_spi_data");
const std::string STR_FRC_AcknowledgedBroadcastBits("acknowledged_broadcast_bits");
const std::string STR_FRC_Temperature("temperature");
const std::string STR_FRC_AcknowledgedBroadcastBytes("acknowledged_broadcast_bytes");
const std::string STR_FRC_MemoryRead("memory_read");
const std::string STR_FRC_MemoryReadPlus1("memory_read_plus1");
const std::string STR_FRC_FrcResponseTime("frc_response_time");

void PrfFrc::init()
{
  m_request.SetLength(sizeof(TDpaIFaceHeader) + sizeof(uint8_t));
  setTimeout(FRC_DEFAULT_TIMEOUT);
}

PrfFrc::PrfFrc()
  :DpaTask(PRF_NAME, PNUM_FRC)
{
  init();
  setUserData({ 0,0 });
}

PrfFrc::PrfFrc(Cmd command, FrcType frcType, uint8_t frcUser, UserData udata)
  : DpaTask(PRF_NAME, PNUM_FRC, 0, (uint8_t)command)
  , m_frcType(frcType)
  , m_frcOffset(frcUser)
{
  init();
  setFrcCommand(frcType, frcUser);
  setUserData(udata);
}

PrfFrc::PrfFrc(Cmd command, FrcCmd frcCmd, UserData udata)
  : DpaTask(PRF_NAME, PNUM_FRC, 0, (uint8_t)command)
{
  init();
  setFrcCommand(frcCmd);
  setUserData(udata);
}

PrfFrc::~PrfFrc()
{
}

uint8_t PrfFrc::getFrcCommand() const
{
  return m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.FrcCommand;
}

void PrfFrc::setFrcCommand(FrcCmd frcCmd)
{
  if ((uint8_t)frcCmd <= FRC_USER_BIT_TO) {
    m_frcType = FrcType::GET_BIT2;
    m_frcOffset = (uint8_t)frcCmd;
  }
  else if ((uint8_t)frcCmd <= FRC_USER_BYTE_TO) {
    m_frcType = FrcType::GET_BYTE;
    m_frcOffset = (uint8_t)frcCmd - FRC_USER_BIT_TO + 1;
  }
  else {
    m_frcType = FrcType::GET_BYTE2;
    m_frcOffset = (uint8_t)frcCmd - FRC_USER_BYTE_TO + 1;
  }

  m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.FrcCommand = (uint8_t)frcCmd;
}

void PrfFrc::setFrcCommand(FrcType frcType, uint8_t frcUser)
{
  if ((frcType == FrcType::GET_BIT2 && frcUser > FRC_USER_BIT_TO - FRC_USER_BIT_FROM) ||
    (frcType == FrcType::GET_BYTE && frcUser > FRC_USER_BYTE_TO - FRC_USER_BYTE_FROM) ||
    (frcType == FrcType::GET_BYTE2 && frcUser > FRC_USER_2BYTE_TO - FRC_USER_2BYTE_FROM)) {
    THROW_EX(std::logic_error, "Inappropriate: " <<
      NAME_PAR(FrcType, encodeFrcType(frcType)) << NAME_PAR(FrcUser, (int)frcUser));
  }
  else {
    m_frcType = frcType;
    m_frcOffset = frcUser;
    m_request.DpaPacket().DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.FrcCommand = (uint8_t)m_frcType + m_frcOffset;
  }
}

uint8_t PrfFrc::getFrcData_bit2(uint16_t addr) const
{
  if (m_frcType != FrcType::GET_BIT2) {
    TRC_WAR("Beware inconsistent call of the method: " << NAME_PAR(FrcType, encodeFrcType(m_frcType)));
  }
  uint8_t retval = 0;
  if (addr <= FRC_MAX_NODE_BIT2 && addr > 0)
    retval = m_data[addr - 1];
  return retval;
}

uint8_t PrfFrc::getFrcData_Byte(uint16_t addr) const
{
  if (m_frcType != FrcType::GET_BYTE) {
    TRC_WAR("Beware inconsistent call of the method: " << NAME_PAR(FrcType, encodeFrcType(m_frcType)));
  }
  uint8_t retval = 0;
  if (addr <= FRC_MAX_NODE_BYTE && addr > 0)
    retval = m_data[addr-1];
  return retval;
}

uint16_t PrfFrc::getFrcData_Byte2(uint16_t addr) const
{
  if (m_frcType != FrcType::GET_BYTE2) {
    TRC_WAR("Beware inconsistent call of the method: " << NAME_PAR(FrcType, encodeFrcType(m_frcType)));
  }
  uint16_t retval = 0;
  if (addr <= FRC_MAX_NODE_BYTE2 && addr > 0)
    retval = *((uint16_t*)&m_data[(addr-1)*2]);
  return retval;
}

void PrfFrc::setUserData(const UserData& udata)
{
  if (udata.size() < FRC_MIN_UDATA_LEN || udata.size() > FRC_MAX_UDATA_LEN)
    THROW_EX(std::logic_error, "Bad user data size: " << NAME_PAR(size, udata.size()));

  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  switch (getCmd()) {
  case Cmd::SEND: {
    packet.DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.FrcCommand = (uint8_t)m_frcType + m_frcOffset;
    std::copy(udata.begin(), udata.end()-1, packet.DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.UserData);
    m_request.SetLength(m_request.GetLength() + udata.size());

    break;
  }
  case Cmd::EXTRARESULT:
  case Cmd::SEND_SELECTIVE:
  case Cmd::SET_PARAMS:
  default:
    m_request.SetLength(sizeof(TDpaIFaceHeader) + 3);
  }
}

void PrfFrc::setDpaTask(const DpaTask& dpaTask)
{
  DpaMessage msg = dpaTask.getRequest();
}

void PrfFrc::parseResponse(const DpaMessage& response)
{
  //TODO missing Extra result processing
  bool extraResult = false;

  if (getCmd() == Cmd::SEND) {
    m_status = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response.Status;

    switch (m_frcType) {

    case FrcType::GET_BIT2:
    {
      int resulSize = extraResult ? 30 : 23;
      TPerFrcSend_Response rsp = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response;

      const uint8_t* pdata0 = &(rsp.FrcData[0]);
      const uint8_t* pdata1 = &(rsp.FrcData[32]);
      uint8_t val0, val1, val;
      int i, ii, iii;
      for (i = 0; i < resulSize; i++) {
        val0 = *(pdata0 + i);
        val1 = *(pdata1 + i);
        for (ii = 0; ii < 8; ii++) {
          val = val1 & 0x01;
          val <<= 1;
          val |= val0 & 0x01;
          val0 >>= 1;
          val1 >>= 1;
          if (iii = 8*i + ii) //skip 0 node
            m_data[iii-1] = val;
        }
      }
    }
    break;

    case FrcType::GET_BYTE:
    {
      int resulSize = extraResult ? 62 : 54;
      const uint8_t* pdata = &(response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response.FrcData[1]);
      for (int i = 0; i < resulSize; i++) {
        m_data[i] = *(pdata + i);
      }
    }
    break;

    case FrcType::GET_BYTE2:
    {
      int resulSize = extraResult ? 30 : 26;
      resulSize *= 2;
      TPerFrcSend_Response rsp = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response;
      const uint8_t* pdata = &(rsp.FrcData[2]);
      for (int i = 0; i < resulSize; i += 2) {
        m_data[i] = *(pdata + i);
        m_data[i + 1] = *(pdata + i + 1);
      }
    }
    break;

    default:
      ;
    }
  }
}

PrfFrc::Cmd PrfFrc::getCmd() const
{
  return m_cmd;
}

void PrfFrc::setCmd(PrfFrc::Cmd cmd)
{
  m_cmd = cmd;
  setPcmd((uint8_t)m_cmd);
}

void PrfFrc::parseCommand(const std::string& command)
{
  if (STR_CMD_SEND == command)
    setCmd(Cmd::SEND);
  else if (STR_CMD_EXTRARESULT == command)
    setCmd(Cmd::EXTRARESULT);
  else if (STR_CMD_SEND_SELECTIVE == command)
    setCmd(Cmd::SEND_SELECTIVE);
  else if (STR_CMD_SET_PARAMS == command)
    setCmd(Cmd::SET_PARAMS);
  else
    THROW_EX(std::logic_error, "Invalid command: " << PAR(command));
}

const std::string& PrfFrc::encodeCommand() const
{
  switch (getCmd()) {
  case Cmd::SEND: return STR_CMD_SEND;
  case Cmd::EXTRARESULT: return STR_CMD_EXTRARESULT;
  case Cmd::SEND_SELECTIVE: return STR_CMD_SEND_SELECTIVE;
  case Cmd::SET_PARAMS: return STR_CMD_SET_PARAMS;
  default:
    THROW_EX(std::logic_error, "Invalid command: " << NAME_PAR(command, (int)getCmd()));
  }
}

PrfFrc::FrcType PrfFrc::parseFrcType(const std::string& frcType)
{
  if (STR_GET_BIT2 == frcType)
    return FrcType::GET_BIT2;
  else if (STR_GET_BYTE == frcType)
    return FrcType::GET_BYTE;
  else if (STR_GET_BYTE2 == frcType)
    return FrcType::GET_BYTE2;
  else
    THROW_EX(std::logic_error, "Invalid: " << NAME_PAR(FrcType, frcType));
}

const std::string& PrfFrc::encodeFrcType(FrcType frcType)
{
  switch (frcType) {
  case FrcType::GET_BIT2: return STR_GET_BIT2;
  case FrcType::GET_BYTE: return STR_GET_BYTE;
  case FrcType::GET_BYTE2: return STR_GET_BYTE2;
  default:
    THROW_EX(std::logic_error, "Invalid: " << NAME_PAR(FrcType, (int)frcType));
  }
}

PrfFrc::FrcCmd PrfFrc::parseFrcCmd(const std::string& frcCmd)
{
  if (STR_FRC_Prebonding == frcCmd)
    return FrcCmd::Prebonding;
  else if (STR_FRC_UART_SPI_data == frcCmd)
    return FrcCmd::UART_SPI_data;
  else if (STR_FRC_AcknowledgedBroadcastBits == frcCmd)
    return FrcCmd::AcknowledgedBroadcastBits;
  else if (STR_FRC_Temperature == frcCmd)
    return FrcCmd::Temperature;
  else if (STR_FRC_AcknowledgedBroadcastBytes == frcCmd)
    return FrcCmd::AcknowledgedBroadcastBytes;
  else if (STR_FRC_MemoryRead == frcCmd)
    return FrcCmd::MemoryRead;
  else if (STR_FRC_MemoryReadPlus1 == frcCmd)
    return FrcCmd::MemoryReadPlus1;
  else if (STR_FRC_FrcResponseTime == frcCmd)
    return FrcCmd::FrcResponseTime;
  else
    THROW_EX(std::logic_error, "Invalid: " << NAME_PAR(FrcCmd, frcCmd));
}

const std::string& PrfFrc::encodeFrcCmd(FrcCmd frcCmd)
{
  switch (frcCmd) {
  case FrcCmd::Prebonding: return STR_FRC_Prebonding;
  case FrcCmd::UART_SPI_data: return STR_FRC_UART_SPI_data;
  case FrcCmd::AcknowledgedBroadcastBits: return STR_FRC_AcknowledgedBroadcastBits;
  case FrcCmd::Temperature: return STR_FRC_Temperature;
  case FrcCmd::AcknowledgedBroadcastBytes: return STR_FRC_AcknowledgedBroadcastBytes;
  case FrcCmd::MemoryRead: return STR_FRC_MemoryRead;
  case FrcCmd::MemoryReadPlus1: return STR_FRC_MemoryReadPlus1;
  case FrcCmd::FrcResponseTime: return STR_FRC_FrcResponseTime;
  default:
    THROW_EX(std::logic_error, "Invalid: " << NAME_PAR(FrcCmd, (int)frcCmd));
  }
}
