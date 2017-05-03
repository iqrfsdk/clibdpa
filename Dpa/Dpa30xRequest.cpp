#include "Dpa30xRequest.h"

Dpa30xRequest::Dpa30xRequest(DpaTransaction* dpaTransaction)
	: DpaRequest(dpaTransaction)
{
}

Dpa30xRequest::~Dpa30xRequest()
{
}

int32_t Dpa30xRequest::EstimateStdTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot)
{
	auto estimated_timeout_ms = (hops + 1) * timeslot * 10;
	int32_t response_time_slot_length_ms;
	if (timeslot == 20)
	{
		response_time_slot_length_ms = 200;
	}
	else
	{
		response_time_slot_length_ms = 60;
	}
	estimated_timeout_ms += (hops_response + 1) * response_time_slot_length_ms + 40;
	return estimated_timeout_ms;
}

int32_t Dpa30xRequest::EstimateLpTimeout(uint8_t hops, uint8_t hops_response, uint8_t timeslot)
{
	auto estimated_timeout_ms = (hops + 1) * timeslot * 10;
	int32_t response_time_slot_length_ms;
	if (timeslot == 20)
	{
		response_time_slot_length_ms = 200;
	}
	else
	{
		response_time_slot_length_ms = 110;
	}
	estimated_timeout_ms += (hops_response + 1) * response_time_slot_length_ms + 40;
	return estimated_timeout_ms;
}
