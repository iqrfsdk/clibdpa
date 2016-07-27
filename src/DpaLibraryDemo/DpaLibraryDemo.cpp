#include <DpaHandler.h>
#include <iostream>
#include <string.h>
#include <iomanip>
#include <thread>
#include <SpiDpaInterface.h>
#include <sysfs_gpio.h>
#include "DpaLibraryDemo.h"
#include "CdcDpaInterface.h"

DpaLibraryDemo* demo_;
CDCImpl* cdc_parser_;
CdcDpaInterface* communication_interface_;

void asyncMsgListener(unsigned char* data, unsigned int length);

void ShowModuleInfo();
int CdcDemoMain();
int SpiDpaDemoMain();

int main(void) {
  std::cout << "Start demo\n";
  //CdcDemoMain();
  SpiDpaDemoMain();
  return 0;
}

int CdcDemoMain() {
  cdc_parser_ = nullptr;

  try {
	cdc_parser_ = new CDCImpl("/dev/ttyACM0");
	bool test = cdc_parser_->test();

	if (test) {
	  std::cout << "Test OK\n";
	} else {
	  std::cout << "Test FAILED\n";
	  delete cdc_parser_;
	  return 2;
	}
  } catch (CDCImplException& e) {
	std::cout << e.getDescr() << "\n";
	if (cdc_parser_ != NULL) {
	  delete cdc_parser_;
	}
	return 1;
  }

  communication_interface_ = new CdcDpaInterface();
  communication_interface_->Init(cdc_parser_);
  demo_ = new DpaLibraryDemo(communication_interface_);

  ShowModuleInfo();

  cdc_parser_->registerAsyncMsgListener(&asyncMsgListener);

  demo_->Start();

  std::cout << "That's all for today...";

  delete communication_interface_;
  delete demo_;
}

/** Reset GPIO. */
#define RESET_GPIO (23)
/** SPI CE GPIO. */
#define RPIIO_PIN_CE0 (8)

int SpiDpaDemoMain() {
  SpiDpaInterface dpaInterface;

  //We prepared demo for Rasperry Pi3
  //enable CE0 for TR communication
  auto initResult = gpio_setup(RPIIO_PIN_CE0, GPIO_DIRECTION_OUT, 0);
  if (initResult < 0) {
	return -1;
  }

  // enable PWR for TR communication
  initResult = gpio_setup(RESET_GPIO, GPIO_DIRECTION_OUT, 1);
  if (initResult < 0) {
	gpio_cleanup(RPIIO_PIN_CE0);
	return -1;
  }

  try {
	dpaInterface.Open("/dev/spidev0.0");
	demo_ = new DpaLibraryDemo(&dpaInterface);
	demo_->Start();
  } catch (...) {
	// clean the mess in all cases
  }

  dpaInterface.Close();
  delete demo_;

  // destroy used rpi_io library
  gpio_cleanup(RESET_GPIO);
  gpio_cleanup(RPIIO_PIN_CE0);

}

void asyncMsgListener(unsigned char* data, unsigned int length) {
  demo_->ListenerWrapper(data, length);
}

void ShowModuleInfo() {
  ModuleInfo* moduleInfo = nullptr;
  moduleInfo = cdc_parser_->getTRModuleInfo();

  std::cout << "Module Info:\n";
  std::cout << "Serial Number: ";
  for (int i = 0; i < ModuleInfo::SN_SIZE; ++i) {
	std::cout << std::hex
			  << std::uppercase
			  << std::setw(2)
			  << std::setfill('0')
			  << (int) moduleInfo->serialNumber[i] << ' ';
  }
  std::cout << '\n';
  std::cout << "OS version: " << std::hex << moduleInfo->osVersion << '\n';

  delete (moduleInfo);
}

DpaLibraryDemo::DpaLibraryDemo(DpaInterface* communication_interface)
	: dpa_handler_(nullptr) {
  dpaInterface_ = communication_interface;
}

DpaLibraryDemo::~DpaLibraryDemo() {
  delete dpa_handler_;
}

void DpaLibraryDemo::Start() {
  try {
	dpa_handler_ = new DpaHandler(dpaInterface_);
	dpa_handler_->RegisterAsyncMessageHandler(std::bind(&DpaLibraryDemo::UnexpectedMessage,
														this,
														std::placeholders::_1));
  }
  catch (std::invalid_argument& ae) {
	std::cout << "There was an error during DPA handler creation.\n";
  }

  dpa_handler_->Timeout(10);    // Default timeout is infinite

  int16_t i = 100;
  //wait for a while, there could be some unread message in CDC
  std::this_thread::sleep_for(std::chrono::seconds(1));

  while (i--) {
	PulseLed(0x00, kLedRed);    // Pulse with red led on coordinator
	PulseLed(0x00, kLedGreen);    // Pulse with green led on coordinator
	PulseLed(0x03, kLedRed);    // Pulse with red led on node with address 3
	PulseLed(0x03, kLedGreen);    // Pulse with green led on node with address 3
	PulseLed(0xFF, kLedRed);    // Pulse with red led on node with address 3
	PulseLed(0xFF, kLedGreen);    // Pulse with green led on node with address 3
	ReadTemperature(0x03);        // Get temperature from node with address 3
  }
}

void DpaLibraryDemo::ListenerWrapper(unsigned char* data, unsigned int length) {
  communication_interface_->ReceiveData(data, length);
}

void DpaLibraryDemo::PulseLed(uint16_t address, LedColor color) {
  DpaMessage::DpaPacket_t packet;
  packet.DpaRequestPacket_t.NADR = address;
  if (color == kLedRed)
	packet.DpaRequestPacket_t.PNUM = PNUM_LEDR;
  else
	packet.DpaRequestPacket_t.PNUM = PNUM_LEDG;

  packet.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  DpaMessage message;
  message.AddDataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

  ExecuteCommand(message);
}

void DpaLibraryDemo::ExecuteCommand(DpaMessage& message) {
  static uint16_t sent_messages = 0;
  static uint16_t timeouts = 0;
  try {
	dpa_handler_->SendDpaMessage(message);
  }
  catch (std::logic_error& le) {
	std::cout << "Send error occured.\n";
	return;
  }

  ++sent_messages;

  while (dpa_handler_->IsDpaMessageInProgress()) {
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (dpa_handler_->Status() == DpaRequest::DpaRequestStatus::kTimeout) {
	++timeouts;
	std::cout << message.NodeAddress()
			  << " - Timeout ..."
			  << sent_messages
			  << ':'
			  << timeouts
			  << '\n';
  }

}
void DpaLibraryDemo::UnexpectedMessage(const DpaMessage& message) {
  std::cout << "Unexpected message received.\n";
}

void DpaLibraryDemo::ReadTemperature(uint16_t address) {
  DpaMessage::DpaPacket_t packet;
  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;

  packet.DpaRequestPacket_t.PCMD = CMD_THERMOMETER_READ;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  DpaMessage message;
  message.AddDataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

  ExecuteCommand(message);

  if (dpa_handler_->Status() == DpaRequest::DpaRequestStatus::kProcessed) {
	int16_t temperature =
		dpa_handler_->CurrentRequest().ResponseMessage().DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
	std::cout << "Temperature: "
			  << std::dec << temperature << " °C\n";
  }
}
