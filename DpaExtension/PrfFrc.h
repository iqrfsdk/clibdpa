#pragma once

#include "DpaTask.h"
#include <bitset>

class PrfFrc : public DpaTask
{
public:
  enum class Cmd {
    SEND = CMD_FRC_SEND,
    EXTRARESULT = CMD_FRC_EXTRARESULT,
    SEND_SELECTIVE = CMD_FRC_SEND_SELECTIVE,
    SET_PARAMS = CMD_FRC_SET_PARAMS
  };

  // Type of collected data
  enum class FrcType {
    GET_BIT2 = FRC_USER_BIT_FROM,
    GET_BYTE = FRC_USER_BYTE_FROM,
    GET_BYTE2 = FRC_USER_2BYTE_FROM
  };

  // Predefined FRC commands
  enum class FrcCmd {
    Prebonding = FRC_Prebonding,
    UART_SPI_data = FRC_UART_SPI_data,
    AcknowledgedBroadcastBits = FRC_AcknowledgedBroadcastBits,
    Temperature = FRC_Temperature,
    AcknowledgedBroadcastBytes = FRC_AcknowledgedBroadcastBytes,
    MemoryRead = FRC_MemoryRead,
    MemoryReadPlus1 = FRC_MemoryReadPlus1,
    FrcResponseTime = FRC_FrcResponseTime
  };

  static const std::string PRF_NAME;

  static const int FRC_MAX_NODE_BIT2 = 239;
  static const int FRC_MAX_NODE_BYTE = 62;
  static const int FRC_MAX_NODE_BYTE2 = 30;

  static const int FRC_MIN_UDATA_LEN = 2;
  static const int FRC_MAX_UDATA_LEN = 30;
  
  static const int FRC_DEFAULT_TIMEOUT = 2000;

  typedef std::basic_string<unsigned char> UserData;

  PrfFrc();
  PrfFrc::PrfFrc(Cmd command, FrcType frcType, uint8_t frcUser, UserData udata = { 0, 0 });
  PrfFrc::PrfFrc(Cmd command, FrcCmd frcCmd, UserData udata = { 0, 0 });
  virtual ~PrfFrc();

  void parseResponse(const DpaMessage& response) override;

  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  Cmd getCmd() const;
  void setCmd(Cmd cmd);

  uint8_t getFrcCommand() const;
  void setFrcCommand(FrcCmd frcCmd);
  void setFrcCommand(FrcType frcType, uint8_t frcUser);
  FrcType getFrcType() const { return m_frcType; }
  uint8_t getFrcUser() const { return m_frcOffset; }

  uint8_t getFrcData_bit2(uint16_t addr) const;
  uint8_t getFrcData_Byte(uint16_t addr) const;
  uint16_t getFrcData_Byte2(uint16_t addr) const;

  void setUserData(const UserData& udata);
  void setDpaTask(const DpaTask& dpaTask); //hosted DpaRequest

protected:
  static FrcType parseFrcType(const std::string& frcType);
  static const std::string& encodeFrcType(FrcType frcType);
  static PrfFrc::FrcCmd parseFrcCmd(const std::string& frcCmd);
  static const std::string& encodeFrcCmd(FrcCmd frcCmd);

private:
  void init();
  Cmd m_cmd = Cmd::SEND;
  FrcType m_frcType = FrcType::GET_BIT2;
  uint8_t m_frcOffset = 0;
  uint8_t m_status = 0;

  uint8_t m_data[FRC_MAX_NODE_BIT2] = { 0 };

};
