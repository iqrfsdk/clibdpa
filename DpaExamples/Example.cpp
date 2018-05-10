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
//#include "DpaTransactionTask.h"
//#include "DpaRaw.h"
//#include "PrfLeds.h"
//#include "PrfFrc.h"
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
	string portNameLinuxSpi = "/dev/spidev0.0";
	spi_iqrf_config_struct spiCfg;
	spiCfg.enableGpioPin = 1;

	// IQRF channel
	IChannel *iqrfChannel;

	try {
		TRC_INF("Creating IQRF interface");
		iqrfChannel = new IqrfCdcChannel(portNameWinCdc);
		//iqrfChannel = new IqrfSpiChannel();
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
		dpaHandler->setTimeout(200);
	}
	catch (std::exception &e) {
		CATCH_EX("Cannot create DpaHandler Interface: ", std::exception, e);
		delete iqrfChannel;
		return 2;
	}

	if (dpaHandler) {
		try {
			// DPA message
			DpaMessage dpaRequest;
			// coordinator address
			dpaRequest.DpaPacket().DpaRequestPacket_t.NADR = 0x00;
			dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = PNUM_LEDG;
			dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
			dpaRequest.DpaPacket().DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;
			// Set request data length
			dpaRequest.SetLength(sizeof(TDpaIFaceHeader));
			// Send DPA request
			std::shared_ptr<IDpaTransaction2> dt = dpaHandler->executeDpaTransaction(dpaRequest, 200);
			auto res = dt->get();
			const uint8_t *buf = res->getResponse().DpaPacket().Buffer;
			int sz = res->getResponse().GetLength();
			for (int i = 0; i < sz; i++)
				cout << std::hex << (int)buf[i] << ' ';
		}
		catch (std::exception& e) {
			CATCH_EX("Error in ExecuteDpaTransaction: ", std::exception, e);
		}
	}
	else {
		TRC_ERR("Dpa interface is not working");
	}

	this_thread::sleep_for(chrono::seconds(10));
	TRC_INF("Clean after yourself");
	delete iqrfChannel;
	delete dpaHandler;

	TRC_LEAVE("");
	TRC_STOP();

	return 0;
}
