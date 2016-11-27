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
  PrfThermometer(int address, Cmd command);
  virtual ~PrfThermometer();

  const DpaMessage& getRequest() override;
  void parseResponse(const DpaMessage& response) override;

  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  int getIntTemperature() const { return m_intTemperature; }
  float getFloatTemperature() const { return m_floatTemperature; }
  uint8_t getRaw8Temperature() const { return m_8Temperature; }
  uint16_t getRaw16Temperature() const { return m_16Temperature; }

private:
  int m_intTemperature = -273;
  float m_floatTemperature = -273.15f;
  uint8_t m_8Temperature = 0;
  uint16_t m_16Temperature = 0;
};
