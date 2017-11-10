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

#pragma once

#include <string>

class DpaTransactionResult {
public:
  DpaTransactionResult() = delete;
  DpaTransactionResult(int errorCode);

  virtual ~DpaTransactionResult();


  /** 
   * Returns code of error.
   * 0: success, -1: DpaHandler timeout, -2: future timeout, <n>: response code
   */
  int getErrorCode() const;

  /**
   * Returns string, which describes error in more detail. 
   */
  std::string getErrorString() const;

private:
  int m_errorCode;
};
