/**
 * Copyright 2015-2017 MICRORISC s.r.o.
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

#include "DpaHandler.h"
#include "DpaTask.h"
#include "PrfThermometer.h"
#include "PrfLeds.h"
#include "DpaLibraryDemo.h"
#include "unexpected_peripheral.h"
#include "IqrfCdcChannel.h"
#include "IqrfSpiChannel.h"
#include "IqrfLogging.h"

#include <iostream>
#include <string.h>
#include <iomanip>
#include <thread>

/**
* Demo app for testing DPA library.
*
* @author Radek Kuchta, Jaroslav Kadlec
* @author Frantisek Mikulu, Rostislav Spinar
*/

TRC_INIT();

int main(int argc, char** argv)
{
  std::string port_name;

  if (argc < 2) {
    std::cerr << "Usage" << std::endl;
    std::cerr << "  DpaDemo <port-name>" << std::endl << std::endl;
    std::cerr << "Example" << std::endl;
    std::cerr << "  DpaDemo COM5" << std::endl;
    std::cerr << "  DpaDemo /dev/ttyACM0" << std::endl;
    std::cerr << "  DpaDemo /dev/spidev0.0" << std::endl;
    return (-1);
  }
  else {
    port_name = argv[1];
  }
  std::cout << "Start demo app ...\n";

  IChannel* dpaInterface(nullptr);
  size_t found = port_name.find("spi");

  if (found != std::string::npos) {
    try {
      dpaInterface = new IqrfSpiChannel(port_name);
    }
    catch (SpiChannelException& e) {
      std::cout << e.what() << std::endl;
      return (-1);
    }
  }
  else {
    try {
      //make a default val here if necessary "/dev/ttyACM0";
      dpaInterface = new IqrfCdcChannel(port_name);
    }
    catch (unexpected_peripheral& e) {
      std::cout << e.what() << std::endl;
      return (-1);
    }
    catch (std::exception& e) {
      std::cout << e.what() << std::endl;
      return (-1);
    }
  }

  // start the app
  DpaLibraryDemo* demoApp = new DpaLibraryDemo(dpaInterface);
  //demoApp->start();
  demoApp->communicationTest();
  std::cout << "That's all for today...";

  // clean
  delete demoApp;
  return 0;
}

/**
 * Constructor
 * @param communicationInterface IQRF interface
 */
DpaLibraryDemo::DpaLibraryDemo(IChannel* communicationInterface)
  : m_dpaHandler(nullptr) {
  m_dpaInterface = communicationInterface;
}

/**
* Destructor
*/
DpaLibraryDemo::~DpaLibraryDemo() {
  delete m_dpaHandler;
}


/**
* App logic
*/
void DpaLibraryDemo::start() {
  try {
    m_dpaHandler = new DpaHandler(m_dpaInterface);
	m_dpaHandler->RegisterAsyncMessageHandler(std::bind(&DpaLibraryDemo::unexpectedMessage,
      this,
      std::placeholders::_1));
  }
  catch (std::invalid_argument& ae) {
    std::cout << "There was an error during DPA handler creation: " << ae.what() << std::endl;
  }

  // default timeout is infinite
  m_dpaHandler->Timeout(100);

  int16_t i = 100;
  //wait for a while, there could be some unread message in CDC
  std::this_thread::sleep_for(std::chrono::seconds(1));

  while (i--) {
	pulseLedRDpaTransaction(0x00);		// Pulse with red led on coordinator
	//pulseLedRDpaTransaction(0x01);	// Pulse with red led on node1

	readTemperatureDpaTransaction(0x00);	// Get temperature from node0
    //ReadTemperatureDpaTransaction(0x01);	// Get temperature from node1

	//PulseLed(0x00, kLedRed);		// Pulse with red led on coordinator
	//PulseLed(0x00, kLedGreen);    // Pulse with green led on coordinator
	//PulseLed(0xFF, kLedRed);		// Pulse with red led using broadcast
	//PulseLed(0xFF, kLedGreen);    // Pulse with green led using broadcast

	//ReadTemperature(0x00);        // Get temperature from coordinator
	//ReadTemperature(0x01);        // Get temperature from node1

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}


/**
* Pulsing LedR on the node using DPA transaction
* @param address Node address
*/
void DpaLibraryDemo::pulseLedRDpaTransaction(uint16_t address)
{
	PrfLedR pulse((int)address, PrfLed::Cmd::PULSE);
	DpaTransactionTask transaction(pulse);

	m_dpaHandler->ExecuteDpaTransaction(transaction);
	int errorCode = transaction.waitFinish();

	if (errorCode == 0)
		std::cout << pulse.getPrfName() << " " << pulse.getAddress() << " " << pulse.encodeCommand() << std::endl;
	else
		std::cout << transaction.getErrorStr() << std::endl;
}


/**
 * Reading temperature from the node using DPA transaction
 * @param address Node address
 */
void DpaLibraryDemo::readTemperatureDpaTransaction(uint16_t address)
{
	PrfThermometer thermometer((int)address, PrfThermometer::Cmd::READ);
	DpaTransactionTask transaction(thermometer);

	m_dpaHandler->ExecuteDpaTransaction(transaction);
	int errorCode = transaction.waitFinish();

	if (errorCode == 0)
		std::cout << NAME_PAR(Temperature, thermometer.getIntTemperature()) << std::endl;
	else
		std::cout << "Failed to read Temperature at: " << NAME_PAR(addres, thermometer.getAddress()) << PAR(errorCode) << std::endl;
}


/**
 * Pulsing LedR on the node using DPA transaction
 * @param address Node address
 */
void DpaLibraryDemo::pulseLed(uint16_t address, LedColor color) {
  DpaMessage::DpaPacket_t packet;

  packet.DpaRequestPacket_t.NADR = address;

  if (color == kLedRed)
    packet.DpaRequestPacket_t.PNUM = PNUM_LEDR;
  else
    packet.DpaRequestPacket_t.PNUM = PNUM_LEDG;

  packet.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  DpaMessage message;
  message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

  // sends request and receive response
  executeCommand(message);
}


/**
 * Reading temperature from the node
 * @param address Node address
 */
void DpaLibraryDemo::readTemperature(uint16_t address) {
  static int num(0);
  DpaMessage::DpaPacket_t packet;

  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;

  packet.DpaRequestPacket_t.PCMD = CMD_THERMOMETER_READ;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  // message container
  DpaMessage message;
  message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

  // sends request and receive response
  executeCommand(message);

  // process the response
  if (m_dpaHandler->Status() == DpaRequest::DpaRequestStatus::kProcessed) {
    int16_t temperature =
	m_dpaHandler->CurrentRequest().ResponseMessage().DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;

    std::cout << num++ << " Temperature: "
    << std::dec << temperature << " oC" << std::endl;
  }
}


/**
 * Execute DPA transaction, sends request and waits for response
 * @param Message DPA message
 */
void DpaLibraryDemo::executeCommand(DpaMessage& message) {
	static uint16_t sent_messages = 0;
	static uint16_t timeouts = 0;

	try {
		m_dpaHandler->SendDpaMessage(message);
	}
	catch (std::logic_error& le) {
		std::cout << "Send error occured: " << le.what() << std::endl;
		return;
	}

	++sent_messages;

	while (m_dpaHandler->IsDpaMessageInProgress()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	if (m_dpaHandler->Status() == DpaRequest::DpaRequestStatus::kTimeout) {
		++timeouts;
		std::cout << message.NodeAddress()
			<< " - Timeout ..."
			<< sent_messages
			<< ':'
			<< timeouts
			<< '\n';
	}
}

void DpaLibraryDemo::unexpectedMessage(const DpaMessage& message) {
	std::cout << "Unexpected message received.\n";
}

void DpaLibraryDemo::communicationTest()
{
	try
	{
		m_dpaHandler = new DpaHandler(m_dpaInterface);
		m_dpaHandler->RegisterAsyncMessageHandler(std::bind(&DpaLibraryDemo::unexpectedMessage,
			this,
			std::placeholders::_1));
	}
	catch (std::invalid_argument& ae)
	{
		std::cout << "There was an error during DPA handler creation: " << ae.what() << std::endl;
	}

	m_dpaHandler->Timeout(200); // Default timeout is infinite

	int16_t i = 100;
	//wait for a while, there could be some unread message in CDC
	std::this_thread::sleep_for(std::chrono::seconds(1));

	uint8_t uartData[55];
	for (uint8_t a = 0; a < 55; ++a)
		uartData[a] = i;

		coordinatorGetAddressingInfo();

//	pulseLed(15, kLedGreen);
//	pulseLed(7, kLedGreen);


	//openUart(15, 3);
	//openUart(7, 3);

	while (i--)
	{
		//sendDataToUart(7, 0, uartData, sizeof uartData);
		//sendDataToUart(15, 0, uartData, sizeof uartData);
		pulseLed(11, kLedGreen);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		pulseLed(11, kLedRed);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		/*
		*PulseLed(15, kLedGreen);
		PulseLed(7, kLedGreen);
		PulseLed(8, kLedGreen);*/
		//std::this_thread::sleep_for(std::chrono::seconds(1));

	}
}


void DpaLibraryDemo::openUart(uint16_t address, uint8_t br)
{
	static int num(0);
	DpaMessage::DpaPacket_t packet;
	packet.DpaRequestPacket_t.NADR = address;
	packet.DpaRequestPacket_t.PNUM = PNUM_UART;

	packet.DpaRequestPacket_t.PCMD = CMD_UART_OPEN;
	packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;
	packet.DpaRequestPacket_t.DpaMessage.PerUartOpen_Request.BaudRate = br;

	DpaMessage message;
	message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader) + 1);

	executeCommand(message);

	if (m_dpaHandler->Status() == DpaRequest::DpaRequestStatus::kProcessed)
	{
		if (m_dpaHandler->CurrentRequest().ResponseMessage().DpaPacket().DpaResponsePacket_t.ResponseCode != STATUS_NO_ERROR)
		{
			std::cout << num++ << " UART Open Error." << std::endl;
		}
	}

}

void DpaLibraryDemo::sendDataToUart(uint16_t address, uint8_t timeout, uint8_t* data, uint8_t dataCount)
{
	static int num(0);
	DpaMessage::DpaPacket_t packet;
	packet.DpaRequestPacket_t.NADR = address;
	packet.DpaRequestPacket_t.PNUM = PNUM_UART;

	packet.DpaRequestPacket_t.PCMD = CMD_UART_WRITE_READ;
	packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;
	packet.DpaRequestPacket_t.DpaMessage.PerUartSpiWriteRead_Request.ReadTimeout = timeout;
	uint8_t* databuff = packet.DpaRequestPacket_t.DpaMessage.PerUartSpiWriteRead_Request.WrittenData;
	uint8_t messageSize = 1 + dataCount;
	while (dataCount--)
	{
		*databuff++ = *data++;
	}

	DpaMessage message;
	message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader) + messageSize);

	executeCommand(message);

	if (m_dpaHandler->Status() == DpaRequest::DpaRequestStatus::kProcessed)
	{
	}
}

uint16_t DpaLibraryDemo::coordinatorGetAddressingInfo()
{
	static int num(0);
	DpaMessage::DpaPacket_t packet;
	packet.DpaRequestPacket_t.NADR = 0x00;
	packet.DpaRequestPacket_t.PNUM = PNUM_COORDINATOR;

	packet.DpaRequestPacket_t.PCMD = CMD_COORDINATOR_ADDR_INFO;
	packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

	DpaMessage message;
	message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

	executeCommand(message);

	if (m_dpaHandler->Status() == DpaRequest::DpaRequestStatus::kProcessed)
	{
		return m_dpaHandler->CurrentRequest().ResponseMessage().DpaPacket().DpaResponsePacket_t.DpaMessage.PerCoordinatorAddrInfo_Response.DevNr;
	}
	return 0;
}
