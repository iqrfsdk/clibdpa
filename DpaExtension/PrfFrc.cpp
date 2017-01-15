#include "PrfFrc.h"
#include "IqrfLogging.h"

const std::string PrfFrc::PRF_NAME("FRC");

const std::string STR_CMD_SEND("SEND");
const std::string STR_CMD_EXTRARESULT("EXTRARESULT");
const std::string STR_CMD_SEND_SELECTIVE("SEND_SELECTIVE");
const std::string STR_CMD_SET_PARAMS("SET_PARAMS");

const std::string STR_UNKNOWN("UNKNOWN");

// Type of collected data
const std::string STR_USER_BIT("USER_BIT_FROM");
const std::string STR_USER_BYTE("USER_BIT_FROM");
const std::string STR_USER_2BYTE("USER_BIT_FROM");

PrfFrc::PrfFrc()
  :DpaTask(PRF_NAME)
{
  trimUserData(m_udata);
}

PrfFrc::PrfFrc(int address, Cmd command, FrcType frcType, uint8_t frcUser, UserData udata)
  : DpaTask(PRF_NAME, address, (int)command)
  , m_frcType(frcType)
  , m_frcUser(frcUser)
  , m_udata(udata)
{
  trimFrcUser(frcType, frcUser);
  trimUserData(m_udata);
}

PrfFrc::PrfFrc(int address, Cmd command, uint8_t frc, UserData udata)
  : DpaTask(PRF_NAME, address, (int)command)
  , m_udata(udata)
{
  if (!separateFrc(frc, m_frcType, m_frcUser)) {
    THROW_EX(std::logic_error, "Reserved value: " << PAR(frc));
  }
  trimUserData(m_udata);
}

PrfFrc::~PrfFrc()
{
}

bool PrfFrc::separateFrc(uint8_t frc, FrcType& frcType, uint8_t& frcUser)
{
  bool retval = true;
  if (frc < FRC_USER_BIT_FROM) {
    retval = false;
  }
  else if (frc <= FRC_USER_BIT_TO) {
    frcType = FrcType::USER_BIT;
    frcUser = frc - FRC_USER_BIT_FROM;
  }
  else if (frc < FRC_USER_BYTE_FROM) {
    retval = false;
  }
  else if (frc <= FRC_USER_BYTE_TO) {
    frcType = FrcType::USER_BYTE;
    frcUser = frc - FRC_USER_BYTE_FROM;
  }
  else if (frc < FRC_USER_2BYTE_FROM) {
    retval = false;
  }
  else if (frc <= FRC_USER_2BYTE_TO) {
    frcType = FrcType::USER_2BYTE;
    frcUser = frc - FRC_USER_2BYTE_FROM;
  }
  return retval;
}

void PrfFrc::trimFrcUser(FrcType frcType, uint8_t& frcUser)
{
  switch (frcType) {
  case FrcType::USER_BIT:
    frcUser = frcUser <= FRC_USER_BIT_TO - FRC_USER_BIT_FROM ?
      frcUser : FRC_USER_BIT_TO - FRC_USER_BIT_FROM;
    break;
  case FrcType::USER_BYTE:
    frcUser = frcUser <= FRC_USER_BYTE_TO - FRC_USER_BYTE_FROM ?
      frcUser : FRC_USER_BYTE_TO - FRC_USER_BYTE_FROM;
    break;
  case FrcType::USER_2BYTE:
    frcUser = frcUser <= FRC_USER_2BYTE_TO - FRC_USER_2BYTE_FROM ?
      frcUser : FRC_USER_2BYTE_TO - FRC_USER_2BYTE_FROM;
    break;
  default:
    frcUser = 0;
  }
}

void PrfFrc::trimUserData(UserData& udata)
{
  while (udata.size() < FRC_MIN_UDATA_LEN) {
    udata.push_back(0);
  }
  if (udata.size() > FRC_MAX_UDATA_LEN) {
    udata = udata.substr(0, FRC_MAX_UDATA_LEN - 1);
  }
}

const DpaMessage& PrfFrc::getRequest()
{
  DpaMessage::DpaPacket_t& packet = m_request.DpaPacket();

  packet.DpaRequestPacket_t.NADR = m_address;
  packet.DpaRequestPacket_t.PNUM = PNUM_FRC;
  packet.DpaRequestPacket_t.PCMD = (uint8_t)m_command;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  switch (m_command) {
  case (int)Cmd::SEND: {
    packet.DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.FrcCommand = (uint8_t)m_frcType + m_frcUser;
    //TODO user data
    packet.DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.UserData[0] = 0;
    packet.DpaRequestPacket_t.DpaMessage.PerFrcSend_Request.UserData[1] = 0;

    m_request.SetLength(sizeof(TDpaIFaceHeader) + 3);

    break;
  }
  case (int)Cmd::EXTRARESULT:
  case (int)Cmd::SEND_SELECTIVE:
  case (int)Cmd::SET_PARAMS:
  default:
    m_request.SetLength(sizeof(TDpaIFaceHeader) + 3);
  }

  return m_request;
}

uint16_t PrfFrc::getData(uint16_t addr) const
{
  uint16_t retval = 0;
  if (addr < FRC_MAX_NODE)
    retval = m_data[addr];
  return retval;
}

void PrfFrc::parseResponse(const DpaMessage& response)
{
  if (m_command == CMD_FRC_SEND) {
    uns8 status = response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response.Status;
    size_t sz = DPA_MAX_DATA_LENGTH - sizeof(uns8);

    //TODO 30, 32 as constants? Derived from DPA_MAX_DATA_LENGTH
    switch (m_frcType) {
    
    case FrcType::USER_BIT:
    {
      const uint8_t* pdata1 = &(response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response.FrcData[0]);
      const uint8_t* pdata2 = &(response.DpaPacket().DpaResponsePacket_t.DpaMessage.PerFrcSend_Response.FrcData[32]);
      uint8_t val1, val2, v;
      for (int i = 0; i < 30; i++) {
        val1 = *(pdata1 + i);
        val2 = *(pdata2 + i);
        for (int ii = 0; ii < 8; ii++) {
          v = val1 & 0x80;
          v >>= 1;
          v |= val2 & 0x80;
          v >>= 6;
          m_data[i*ii] = v;
          val1 <<= 1;
          val2 <<= 1;
        }
      }
    }
    break;
    
    case FrcType::USER_BYTE:
      //TODO
      break;
    case FrcType::USER_2BYTE:
      //TODO
      break;
    default:
      ;
    }
  }
}

void PrfFrc::parseCommand(const std::string& command)
{
  m_valid = true;
  if (STR_CMD_SEND == command)
    m_command = CMD_FRC_SEND;
  else if (STR_CMD_EXTRARESULT == command)
    m_command = CMD_FRC_EXTRARESULT;
  else if (STR_CMD_SEND_SELECTIVE == command)
    m_command = CMD_FRC_SEND_SELECTIVE;
  else if (STR_CMD_SET_PARAMS == command)
    m_command = CMD_FRC_SET_PARAMS;
  m_valid = false;
}

const std::string& PrfFrc::encodeCommand() const
{
  switch (m_command) {
  case CMD_FRC_SEND: return STR_CMD_SEND;
  case CMD_FRC_EXTRARESULT: return STR_CMD_EXTRARESULT;
  case CMD_FRC_SEND_SELECTIVE: return STR_CMD_SEND_SELECTIVE;
  case CMD_FRC_SET_PARAMS: return STR_CMD_SET_PARAMS;
  default:
    return STR_UNKNOWN;
  }
}

void PrfFrc::parseFrcType(const std::string& frcType)
{
  m_valid = true;
  if (STR_USER_BIT == frcType)
    m_frcType = FrcType::USER_BIT;
  else if (STR_USER_BYTE == frcType)
    m_frcType = FrcType::USER_BYTE;
  else if (STR_USER_2BYTE == frcType)
    m_frcType = FrcType::USER_2BYTE;
  m_valid = false;
}

const std::string& PrfFrc::encodeFrcType() const
{
  switch (m_frcType) {
  case FrcType::USER_BIT: return STR_USER_BIT;
  case FrcType::USER_BYTE: return STR_USER_BYTE;
  case FrcType::USER_2BYTE: return STR_USER_2BYTE;
  default:
    return STR_UNKNOWN;
  }
}
