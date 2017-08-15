/**
* Copyright 2015-2017 IQRF Tech s.r.o.
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

#include "IqrfCdcChannel.h"
//#include "IqrfSpiChannel.h"
#include "DpaHandler.h"
#include "DpaTransactionTask.h"
#include "PrfRaw.h"

#include "IqrfLogging.h"

using namespace std;

TRC_INIT();

int main(int argc, char** argv) {

	TRC_ENTER("");

	// COM interface
	string portNameWinCdc = "COM3";
	//string portNameLinuxCdc = "/dev/ttyACM0";
	//string portNameLinuxSpi = "/dev/spidev0.0";

	TRC_DBG("Communication interface :" << PAR(portNameWinCdc));

	// IQRF channel
	TRC_INF("Creating IQRF channel");
	IChannel *iqrfChannel;
	iqrfChannel = new IqrfCdcChannel(portNameWinCdc);
	//iqrfchannel = new IqrfSpiChannel(portNameLinuxSpi);

	// DPA handler
	TRC_INF("Creating DPA handler");
	DpaHandler *dpaHandler;
	dpaHandler = new DpaHandler(iqrfChannel);

	// default timeout waiting for response is infinite
	dpaHandler->Timeout(1000);
	// default iqrf communication mode is standard 
	dpaHandler->SetRfCommunicationMode(DpaHandler::kStd);

	// DPA message
	TRC_INF("Creating DPA request to pulse LEDR on coordinator");
	DpaMessage dpaRequest;
	// coordinator address
	dpaRequest.DpaPacket().DpaRequestPacket_t.NADR = 0x00;
	
	// embedded peripheral ledr
	dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = 0x06;
	//dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = PNUM_LEDR;

	// command to run
	dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = 0x03;
	//dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
	
	dpaRequest.DpaPacket().DpaRequestPacket_t.HWPID = 0xFFFF;

	// Raw DPA access
	PrfRaw rawDpa(dpaRequest);

	// DPA transaction task
	TRC_INF("Running DPA transaction");
	DpaTransactionTask dpaTT(rawDpa);
	dpaHandler->ExecuteDpaTransaction(dpaTT);
	uint8_t result = dpaTT.waitFinish();
	TRC_DBG("Result from DPA transaction :" << PAR(result));
	
	if(result == 0)
		TRC_INF("Pulse LEDR done!");

	TRC_INF("Waiting 10s before exiting");
	this_thread::sleep_for(chrono::seconds(10));

	TRC_INF("Clean after yourself");
	delete iqrfChannel;
	delete dpaHandler;

	TRC_LEAVE("");
}
