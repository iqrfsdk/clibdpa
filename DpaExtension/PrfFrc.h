#pragma once

#include "DpaTask.h"

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
    USER_BIT = FRC_USER_BIT_FROM,
    USER_BYTE = FRC_USER_BYTE_FROM,
    USER_2BYTE = FRC_USER_2BYTE_FROM
  };

  static const std::string PRF_NAME;
  static const int FRC_MAX_NODE = 240;
  static const int FRC_MIN_UDATA_LEN = 2;
  static const int FRC_MAX_UDATA_LEN = 30;

  typedef std::basic_string<unsigned char> UserData;

  PrfFrc();
  PrfFrc::PrfFrc(int address, Cmd command, FrcType frcType, uint8_t frcUser, UserData udata = { 0, 0 });
  PrfFrc::PrfFrc(int address, Cmd command, uint8_t frc, UserData udata = { 0, 0 });
  virtual ~PrfFrc();

  const DpaMessage& getRequest() override;
  void parseResponse(const DpaMessage& response) override;

  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  FrcType getFrcType() const { return m_frcType; }
  uint8_t getFrcUser() const { return m_frcUser; }
  uint8_t getFrc() const { return (uint8_t)m_frcType + m_frcUser; }

  uint16_t getData(uint16_t addr) const;

  static bool separateFrc(uint8_t frc, FrcType& frcType, uint8_t& frcUser);
  static void trimFrcUser(FrcType frcType, uint8_t& frcUser);
  static void trimUserData(UserData& udata);

private:
  void parseFrcType(const std::string& frcType);
  const std::string& encodeFrcType() const;

  FrcType m_frcType = FrcType::USER_BIT;
  uint8_t m_frcUser = 0;
  UserData m_udata = { 0, 0 };

  uint8_t m_status = 0;
  uint16_t m_data[FRC_MAX_NODE] = { 0 };
};
