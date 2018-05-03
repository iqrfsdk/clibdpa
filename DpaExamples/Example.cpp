/**
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

#include "IqrfCdcChannel.h"
#include "IqrfSpiChannel.h"
#include "DpaHandler2.h"
#include "DpaTransactionTask.h"
#include "DpaRaw.h"
#include "PrfLeds.h"
#include "PrfFrc.h"
#include "IqrfLogging.h"

using namespace std;
using namespace iqrf;

TRC_INIT();

void asynchronousMessageHandler(const DpaMessage& message) {
	TRC_INF("Received as async msg: " << endl
		<< FORM_HEX((unsigned char *)message.DpaPacketData(), message.GetLength()));
}

int main(int argc, char** argv) {

	TRC_START("log.txt", Level::dbg, 1000000);
	TRC_ENTER("");

	// COM interface
	string portNameWinCdc = "COM5";
	string serviceId = "";
	//string portNameLinuxCdc = "/dev/ttyACM0";
	//string portNameLinuxSpi = "/dev/spidev0.0";

	TRC_DBG("Communication interface :" << PAR(portNameWinCdc));

	// IQRF channel
	IChannel *iqrfChannel;

	try {
		TRC_INF("Creating IQRF interface");
		iqrfChannel = new IqrfCdcChannel(portNameWinCdc);
		//iqrfchannel = new IqrfSpiChannel(portNameLinuxSpi);
	}
	catch (std::exception &e) {
		CATCH_EX("Cannot create IqrfInterface: ", std::exception, e);
		return 1;
	}

	// DPA handler
	DpaHandler2 *dpaHandler;

	try {
		TRC_INF("Creating DPA handler");
		dpaHandler = new DpaHandler2(iqrfChannel);

		// async messages
		dpaHandler->registerAsyncMessageHandler(serviceId, &asynchronousMessageHandler);
		// default iqrf communication mode is standard 
		dpaHandler->setRfCommunicationMode(IDpaHandler2::kStd);

		// default timeout waiting for confirmation is 400ms, no need to call it
		// this timeout is then used for all transaction if not set otherwise for each transaction
		dpaHandler->setTimeout(1000);
	}
	catch (std::exception &e) {
		CATCH_EX("Cannot create DpaHandler Interface: ", std::exception, e);
		delete iqrfChannel;
		return 2;
	}

	/*** 1) Generic Raw DPA access ***/
	TRC_INF("Creating DPA request to run discovery on coordinator");
	cout << "Creating DPA request to run discovery on coordinator" << endl;
	/* Option 1*/

	// DPA message
	DpaMessage dpaRequest;

	// coordinator address
	dpaRequest.DpaPacket().DpaRequestPacket_t.NADR = 0x00;

	// embedded peripheral
	dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = PNUM_LEDG;//PNUM_EEPROM;

															   // command to run
	dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = CMD_LED_PULSE;

	// hwpid
	dpaRequest.DpaPacket().DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

	//dpaRequest.DpaPacket().DpaRequestPacket_t.DpaMessage.Request.PData[0] = 0x00;
	//dpaRequest.DpaPacket().DpaRequestPacket_t.DpaMessage.Request.PData[1] = 0x10;

	// set request data length
	dpaRequest.SetLength(sizeof(TDpaIFaceHeader));

	// Raw DPA access
	DpaRaw rawTask(dpaRequest);

	// default timeout waiting for confirmation or response if communicated directly with coordinator is 400ms
	// default timeout waiting for response is based on estimation from DPA confirmation 
	// sets according to your needs and dpa timing requirements but there is no need if you want default

	// ! discovery command needs infinite timeout since we do not know how long will run !
	rawTask.setTimeout(1000);

	// DPA transaction task
	TRC_INF("Running DPA transaction");
	DpaTransactionTask dpaTT1(rawTask);

	if (dpaHandler) {
		try {
			dpaHandler->executeDpaTransaction(dpaRequest, 5000);
		}
		catch (std::exception& e) {
			CATCH_EX("Error in ExecuteDpaTransaction: ", std::exception, e);
			dpaTT1.processFinish(DpaTransfer::kError);
		}
	}
	else {
		TRC_ERR("Dpa interface is not working");
		dpaTT1.processFinish(DpaTransfer::kError);
	}

	int result = dpaTT1.waitFinish();
	TRC_DBG("Result from DPA transaction: " << PAR(result));
	TRC_DBG("Result from DPA transaction as string: " << PAR(dpaTT1.getErrorStr()));

	//DpaMessage dpaResponse = rawTask.getResponse();

	if (result == 0) {
		// getting response data
		DpaMessage dpaResponse = rawTask.getResponse();
		int discoveredNodes = dpaResponse.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData[0];
		TRC_INF("Discovery done: " << PAR(discoveredNodes));
	}


	TRC_INF("Waiting 30s before exit");
	cout << "Waiting 30s before exit" << endl;
	TRC_INF("Space to test asynchronous messages from IQRF");
	cout << "Space to test asynchronous messages from IQRF" << endl;

	this_thread::sleep_for(chrono::seconds(30));

	TRC_INF("Clean after yourself");
	delete iqrfChannel;
	delete dpaHandler;

	TRC_LEAVE("");
	TRC_STOP();

	return 0;
}
