#pragma once
#include "DpaRequest.h"

class Dpa30xRequest
	: public DpaRequest
{
public:
	Dpa30xRequest(DpaTransaction* dpaTransaction);
	~Dpa30xRequest() override;
protected:
	int32_t EstimateStdTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot, int32_t response = -1) override;
	int32_t EstimateLpTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot, int32_t response = -1) override;
public:
	
};
