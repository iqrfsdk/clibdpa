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

#pragma once

#include "DpaTask.h"
#include <chrono>

class PrfOs : public DpaTask
{
public:
  enum class Cmd {
    READ = CMD_OS_READ,
    RESET = CMD_OS_RESET,
    READ_CFG = CMD_OS_READ_CFG,
    RFPGM = CMD_OS_RFPGM,
    SLEEP = CMD_OS_SLEEP,
    BATCH = CMD_OS_BATCH,
    SET_USEC = CMD_OS_SET_USEC,
    SET_MID = CMD_OS_SET_MID,
    RESTART = CMD_OS_RESTART,
    WRITE_CFG_BYTE = CMD_OS_WRITE_CFG_BYTE,
    LOAD_CODE = CMD_OS_LOAD_CODE,
    WRITE_CFG = CMD_OS_WRITE_CFG
  };

  enum class TimeControl {
    WAKEUP_PB4_NEGATIVE = 1,
    RUN_CALIB = 2,
    LEDG_FLASH = 4,
    WAKEUP_PB4_POSITIVE = 8
  };

  static const std::string PRF_NAME;

  PrfOs();
  PrfOs(uint16_t address);
  virtual ~PrfOs();

  void parseResponse(const DpaMessage& response) override;
  void parseCommand(const std::string& command) override;
  const std::string& encodeCommand() const override;

  void sleep(const std::chrono::seconds& sec, uint8_t ctrl = 0);
  void sleep(const std::chrono::milliseconds& milis, uint8_t ctrl = 0);
  void calibration();

private:
  typedef std::chrono::duration<unsigned long, std::ratio<2097, 1000>> milis2097;
  typedef std::chrono::duration<unsigned long, std::ratio<32768, 1000000>> micros32768;

  Cmd getCmd() const;
  void setCmd(Cmd cmd);
  Cmd m_cmd = Cmd::READ;

  uint16_t m_time = 0;
  uint8_t m_timeCtrl = 0;
};
