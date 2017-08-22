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
#include "DpaHandler.h"
#include "DpaTransactionTask.h"
#include "PrfRaw.h"
#include "PrfLeds.h"
#include "IqrfLogging.h"

using namespace std;
using namespace iqrf;

TRC_INIT();

void asynchronousMessageHandler(const DpaMessage& message) {
  TRC_INF("Asynchronous message recevied");
}

int main(int argc, char** argv) {

  //TRC_START("log.txt", Level::dbg, 1000000);
  TRC_START("", Level::dbg, 1000000);
  TRC_ENTER("");

  // COM interface
  string portNameWinCdc = "COM4";
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

  // async messages
  dpaHandler->RegisterAsyncMessageHandler(&asynchronousMessageHandler);
  // default iqrf communication mode is standard 
  dpaHandler->SetRfCommunicationMode(DpaHandler::kStd);

  /*** 1) Generic Raw DPA access ***/
  TRC_INF("Creating DPA request to pulse LEDR on coordinator");
  /* Option 1*/

    // DPA message
  DpaMessage dpaRequest;
  // coordinator address
  dpaRequest.DpaPacket().DpaRequestPacket_t.NADR = 0x00;

  // embedded peripheral
  dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = 0x06;
  //dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = PNUM_LEDR;

  // command to run
  dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = 0x03;
  //dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = CMD_LED_PULSE;

  // hwpid
  dpaRequest.DpaPacket().DpaRequestPacket_t.HWPID = 0xFFFF;

  // set request data length
  dpaRequest.SetLength(sizeof(TDpaIFaceHeader));

  /* Option 2*/
  /*
    DpaMessage::DpaPacket_t dpaPacket;
    dpaPacket.DpaRequestPacket_t.NADR = 0x00;
    dpaPacket.DpaRequestPacket_t.PNUM = PNUM_LEDR;
    dpaPacket.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
    dpaPacket.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

    dpaRequest.DataToBuffer(dpaPacket.Buffer, sizeof(TDpaIFaceHeader));
  */

  // Raw DPA access
  PrfRaw rawDpa(dpaRequest);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT1(rawDpa);
  // default timeout waiting for response is infinite
  // sets according to your needs and dpa timing requirements
  dpaHandler->Timeout(500);
  dpaHandler->ExecuteDpaTransaction(dpaTT1);
  int result = dpaTT1.waitFinish();
  TRC_DBG("Result from DPA transaction :" << PAR(result));
  TRC_DBG("Result from DPA transaction as string :" << PAR(dpaTT1.getErrorStr()));

  if (result == 0)
    TRC_INF("Pulse LEDR done!");

  TRC_INF("Waiting 5s before next transaction");
  this_thread::sleep_for(chrono::seconds(5));

  /*** 2) Peripheral Ledr DPA access ***/

    // PrfLedr DPA access
  PrfLedR ledrPulseDpa(0x00, PrfLed::Cmd::PULSE);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT2(ledrPulseDpa);
  // default timeout waiting for response is infinite
  // sets according to your needs and dpa timing requirements
  dpaHandler->Timeout(500);
  dpaHandler->ExecuteDpaTransaction(dpaTT2);
  result = dpaTT2.waitFinish();
  TRC_DBG("Result from DPA transaction :" << PAR(result));
  TRC_DBG("Result from DPA transaction as string :" << PAR(dpaTT2.getErrorStr()));

  if (result == 0) {
    TRC_INF("Pulse LEDR done!");
    TRC_DBG("DPA transaction :" << NAME_PAR(ledrPulseDpa.getPrfName(), ledrPulseDpa.getAddress()) << PAR(ledrPulseDpa.encodeCommand()));
  }
  else {
    TRC_DBG("DPA transaction error: " << PAR(result));
  }

  TRC_INF("Waiting 5s before exit");
  this_thread::sleep_for(chrono::seconds(5));

  TRC_INF("Clean after yourself");
  delete iqrfChannel;
  delete dpaHandler;

  TRC_LEAVE("");
  TRC_STOP();
}
