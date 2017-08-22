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

#include "Dpa30xRequest.h"

Dpa30xRequest::Dpa30xRequest(DpaTransaction* dpaTransaction)
  : DpaRequest(dpaTransaction)
{
}

Dpa30xRequest::~Dpa30xRequest()
{
}

int32_t Dpa30xRequest::EstimateStdTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot, int32_t response)
{
  auto estimated_timeout_ms = (hops + 1) * timeslot * 10;
  int32_t response_time_slot_length_ms;
  if (timeslot == 20)
  {
    response_time_slot_length_ms = 200;
  }
  else
  {
    if (response >= 0 && response < 16)
    {
      response_time_slot_length_ms = 40;
    }
    else if (response >= 16 && response < 39)
    {
      response_time_slot_length_ms = 50;
    }
    else
    {
      response_time_slot_length_ms = 60;
    }
  }
  estimated_timeout_ms += (hops_response + 1) * response_time_slot_length_ms + safety_timeout_ms_;
  return estimated_timeout_ms;
}

int32_t Dpa30xRequest::EstimateLpTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot, int32_t response)
{
  auto estimated_timeout_ms = (hops + 1) * timeslot * 10;
  int32_t response_time_slot_length_ms;
  if (timeslot == 20)
  {
    response_time_slot_length_ms = 200;
  }
  else
  {
    if (response >= 0 && response < 11)
    {
      response_time_slot_length_ms = 80;
    }
    else if (response >= 11 && response < 33)
    {
      response_time_slot_length_ms = 90;
    }
    else if (response >= 33 && response < 56)
    {
      response_time_slot_length_ms = 100;
    }
    else
    {
      response_time_slot_length_ms = 110;
    }
  }
  estimated_timeout_ms += (hops_response + 1) * response_time_slot_length_ms + safety_timeout_ms_;
  return estimated_timeout_ms;
}
