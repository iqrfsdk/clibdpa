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
#include "DpaRaw.h"
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
  TRC_START("log.txt", Level::dbg, 1000000);
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
  dpaHandler->SetRfCommunicationMode(kLp);


  /*** 1) Generic Raw DPA access ***/
  TRC_INF("Creating DPA request to run discovery on coordinator");
  /* Option 1*/

  // DPA message
  DpaMessage dpaRequest;

  // coordinator address
  dpaRequest.DpaPacket().DpaRequestPacket_t.NADR = 0x00;

  // embedded peripheral
  dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = 0x00;
  //dpaRequest.DpaPacket().DpaRequestPacket_t.PNUM = PNUM_COORDINATOR;

  // command to run
  dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = 0x07;
  //dpaRequest.DpaPacket().DpaRequestPacket_t.PCMD = CMD_COORDINATOR_DISCOVERY;

  // hwpid
  dpaRequest.DpaPacket().DpaRequestPacket_t.HWPID = 0xFFFF;
  //dpaPacket.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  // tx power for discovery
  dpaRequest.DpaPacket().DpaRequestPacket_t.DpaMessage.PerCoordinatorDiscovery_Request.TxPower = 0x07;

  // max number of nodes to discover (0x00 - whole network)
  dpaRequest.DpaPacket().DpaRequestPacket_t.DpaMessage.PerCoordinatorDiscovery_Request.MaxAddr = 0x00;

  // set request data length
  dpaRequest.SetLength(sizeof(TDpaIFaceHeader) + 2);

  /* Option 2*/
  /*
    DpaMessage::DpaPacket_t dpaPacket;
    dpaPacket.DpaRequestPacket_t.NADR = 0x00;
    dpaPacket.DpaRequestPacket_t.PNUM = PNUM_COORDINATOR;
    dpaPacket.DpaRequestPacket_t.PCMD = CMD_COORDINATOR_DISCOVERY;
    dpaPacket.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;
    dpaRequest.DpaPacket().DpaRequestPacket_t.DpaMessage.PerCoordinatorDiscovery_Request.TxPower = 0x07;
    dpaRequest.DpaPacket().DpaRequestPacket_t.DpaMessage.PerCoordinatorDiscovery_Request.MaxAddr = 0x00;

    dpaRequest.DataToBuffer(dpaPacket.Buffer, sizeof(TDpaIFaceHeader) + 2);
  */

  // Raw DPA access
  DpaRaw rawTask(dpaRequest);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT1(rawTask);

  // default timeout waiting for confirmation is 200ms
  // default timeout waiting for response is based on estimation from DPA confirmation 
  // sets according to your needs and dpa timing requirements but there is no need if you want default
  
  // ! discovery command needs infinite timeout since we do not know how long will run !
  dpaHandler->Timeout(DpaHandler::INFINITE_TIMING);

  dpaHandler->ExecuteDpaTransaction(dpaTT1);
  int result = dpaTT1.waitFinish();
  TRC_DBG("Result from DPA transaction: " << PAR(result));
  TRC_DBG("Result from DPA transaction as string: " << PAR(dpaTT1.getErrorStr()));

  if (result == 0) {
    // getting response data
    DpaMessage dpaResponse = rawTask.getResponse();
    int discoveredNodes = dpaResponse.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData[0];
    TRC_INF("Discovery done: " << PAR(discoveredNodes));
  }

  /*** 2) Peripheral Ledr DPA access ***/

  // PrfLedr DPA access
  PrfLedR ledrPulseDpa(0x0A, PrfLed::Cmd::PULSE);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT2(ledrPulseDpa);
  
  // default timeout waiting for confirmation is 200ms
  // default timeout waiting for response is based on estimation from DPA confirmation 
  // sets according to your needs and dpa timing requirements but there is no need if you want default
  dpaHandler->Timeout(DpaHandler::DEFAULT_TIMING);

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

  TRC_INF("Waiting 3s before exit");
  this_thread::sleep_for(chrono::seconds(3));

  TRC_INF("Clean after yourself");
  delete iqrfChannel;
  delete dpaHandler;

  TRC_LEAVE("");
  TRC_STOP();
}
