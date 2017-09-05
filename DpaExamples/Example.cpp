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
  string portNameWinCdc = "COM4";
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
  DpaHandler *dpaHandler;

  try {
    TRC_INF("Creating DPA handler");
    dpaHandler = new DpaHandler(iqrfChannel);

    // async messages
    dpaHandler->RegisterAsyncMessageHandler(&asynchronousMessageHandler);
    // default iqrf communication mode is standard 
    dpaHandler->SetRfCommunicationMode(kLp);

    // default timeout waiting for confirmation is 400ms, no need to call it
    // this timeout is then used for all transaction if not set otherwise for each transaction
    dpaHandler->Timeout(DpaHandler::DEFAULT_TIMING);
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

  // default timeout waiting for confirmation or response if communicated directly with coordinator is 400ms
  // default timeout waiting for response is based on estimation from DPA confirmation 
  // sets according to your needs and dpa timing requirements but there is no need if you want default

  // ! discovery command needs infinite timeout since we do not know how long will run !
  rawTask.setTimeout(DpaHandler::INFINITE_TIMING);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT1(rawTask);

  if (dpaHandler) {
    try {
      dpaHandler->ExecuteDpaTransaction(dpaTT1);
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

  if (result == 0) {
    // getting response data
    DpaMessage dpaResponse = rawTask.getResponse();
    int discoveredNodes = dpaResponse.DpaPacket().DpaResponsePacket_t.DpaMessage.Response.PData[0];
    TRC_INF("Discovery done: " << PAR(discoveredNodes));
  }

  /*** 2) Peripheral Ledr DPA access ***/
  
  TRC_INF("Creating DPA request to pulse LEDR on all nodes");
  cout << "Creating DPA request to pulse LEDR on all nodes" << endl;

  // PrfLedr DPA access
  PrfLedR ledrPulseDpa(0xFF, PrfLed::Cmd::PULSE);

  // default timeout waiting for confirmation or response if communicated directly with coordinator is 400ms
  // default timeout waiting for response is based on estimation from DPA confirmation 
  // sets according to your needs and dpa timing requirements but there is no need if you want default

  // no need to set default here since it is already set at DpaHandler level
  ledrPulseDpa.setTimeout(DpaHandler::DEFAULT_TIMING);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT2(ledrPulseDpa);

  if (dpaHandler) {
    try {
      dpaHandler->ExecuteDpaTransaction(dpaTT2);
    }
    catch (std::exception& e) {
      CATCH_EX("Error in ExecuteDpaTransaction: ", std::exception, e);
      dpaTT2.processFinish(DpaTransfer::kError);
    }
  }
  else {
    TRC_ERR("Dpa interface is not working");
    dpaTT2.processFinish(DpaTransfer::kError);
  }

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

  /*** 3) Peripheral FRC DPA access ***/

  TRC_INF("Creating DPA request to send FRC to all nodes");
  cout << "Creating DPA request to send FRC to all nodes" << endl;

  // PrfFrc DPA access
  PrfFrc frcDpa(PrfFrc::Cmd::SEND, PrfFrc::FrcCmd::Temperature);

  // default timeout waiting for confirmation or response if communicated directly with coordinator is 400ms
  // default timeout waiting for response is based on estimation from DPA confirmation 
  // sets according to your needs and dpa timing requirements but there is no need if you want default

  // FRC requires its own timing, consult iqrf os guide to set it correctly according to your IQRF network
  frcDpa.setTimeout(5000);

  // DPA transaction task
  TRC_INF("Running DPA transaction");
  DpaTransactionTask dpaTT3(frcDpa);

  if (dpaHandler) {
    try {
      dpaHandler->ExecuteDpaTransaction(dpaTT3);
    }
    catch (std::exception& e) {
      CATCH_EX("Error in ExecuteDpaTransaction: ", std::exception, e);
      dpaTT3.processFinish(DpaTransfer::kError);
    }
  }
  else {
    TRC_ERR("DPA interface is not working");
    dpaTT3.processFinish(DpaTransfer::kError);
  }

  result = dpaTT3.waitFinish();
  TRC_DBG("Result from DPA transaction :" << PAR(result));
  TRC_DBG("Result from DPA transaction as string :" << PAR(dpaTT3.getErrorStr()));

  if (result == 0) {
    TRC_INF("FRC done!");
    TRC_DBG("DPA transaction :" << NAME_PAR(frcDpa.getPrfName(), frcDpa.getAddress()) << PAR(frcDpa.encodeCommand()));
  }
  else {
    TRC_DBG("DPA transaction error: " << PAR(result));
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
