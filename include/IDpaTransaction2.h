/**
* Copyright 2015-2018 MICRORISC s.r.o.
* Copyright 2018 IQRF Tech s.r.o.
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

#include "IDpaTransactionResult2.h"

class IDpaTransaction2
{
public:
  enum RfMode {
    kStd,
    kLp
  };

  enum FrcResponseTime {
    k40Ms = 0x00,
    k360Ms = 0x10,
    k680Ms = 0x20,
    k1320Ms = 0x30,
    k2600Ms = 0x40,
    k5160Ms = 0x50,
    k10280Ms = 0x60,
    k20620Ms = 0x70
  };

  struct FRC_TimingParams
  {
    uint8_t bondedNodes;
    uint8_t discoveredNodes;
    FrcResponseTime responseTime;
  };

  // Timing constants
  /// Default timeout
  static const int DEFAULT_TIMEOUT = 500;
  /// Minimal timeout used if required by user is too low
  static const int MINIMAL_TIMEOUT = 200;
  /// Zero value used to indicate infinit timeout in special cases (discovery)
  static const int INFINITE_TIMEOUT = 0;
  /// An extra timeout added to timeout from a confirmation packet.
  static const int32_t SAFETY_TIMEOUT_MS = 40;
  /// A special DISCOVERY timeout
  static const int32_t BOND_TIMEOUT_MS = 11000;

  virtual ~IDpaTransaction2() {}
  /// wait for result
  virtual std::unique_ptr<IDpaTransactionResult2> get() = 0;
  /// abort the transaction immediately 
  virtual void abort() = 0;
};
