#pragma once

#include "DpaTask.h"

static const std::string  PRF_NAME_Thermometer("Thermometer");

class PrfThermometer : public DpaTask
{
public:
  enum Cmd {
    READ = CMD_THERMOMETER_READ
  };

  PrfThermometer(unsigned short address, int command);
  virtual ~PrfThermometer() {}
  virtual void parseResponse(const DpaMessage& response);
  int getIntTemperature() const { return m_intTemperature; }
  float getFloatTemperature() const { return m_floatTemperature; }
  uint8_t getRaw8Temperature() const { return m_8Temperature; }
  uint16_t getRaw16Temperature() const { return m_16Temperature; }

  virtual void toStream(std::ostream& os) const;
  static int convertCommand(const std::string& command);
  static std::string convertCommand(int command);
private:
  int m_intTemperature;
  float m_floatTemperature;
  uint8_t m_8Temperature;
  uint16_t m_16Temperature;
};
