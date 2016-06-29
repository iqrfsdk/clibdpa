#include <DpaHandler.h>
#include <iostream>
#include <string.h>
#include <iomanip>
#include <thread>
#include "DpaLibraryDemo.h"
#include "CdcDpaInterface.h"

DpaLibraryDemo* demo_;
CDCImpl* cdc_parser_;

void asyncMsgListener(unsigned char* data, unsigned int length);

void ShowModuleInfo();

int main(void) {
  std::cout << "Start demo\n";

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

  CdcDpaInterface* communication_interface = new CdcDpaInterface();
  communication_interface->Init(cdc_parser_);
  demo_ = new DpaLibraryDemo(communication_interface);

  ShowModuleInfo();

  cdc_parser_->registerAsyncMsgListener(&asyncMsgListener);

  demo_->Start();

  std::cout << "That's all for today...";

  delete communication_interface;
  delete demo_;
  return 0;
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
  dpa_handler_ = new DpaHandler(dpaInterface_);

  int16_t i = 1000;

  while (i--) {
	PulseLed(0x00, kLedRed);
	PulseLed(0x00, kLedGreen);
//	PulseLed(0x01, kLedRed);
//	PulseLed(0x01, kLedGreen);
  }
}

void DpaLibraryDemo::ListenerWrapper(unsigned char* data, unsigned int length) {
  dpa_handler_->ResponseHandler(data, length);
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
