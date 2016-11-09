#include <DpaHandler.h>
#include "DpaTask.h"
#include "DpaLibraryDemo.h"
#include "unexpected_peripheral.h"
#include "IqrfCdcChannel.h"
#include "IqrfSpiChannel.h"
#include "IqrfLogging.h"

#include <iostream>
#include <string.h>
#include <iomanip>
#include <thread>

TRC_INIT("");

int main(int argc, char** argv)
{
  std::string port_name;

  if (argc < 2) {
    std::cerr << "Usage" << std::endl;
    std::cerr << "  dpa_demo <port-name>" << std::endl << std::endl;
    std::cerr << "Example" << std::endl;
    std::cerr << "  dpa_demo COM5" << std::endl;
    std::cerr << "  dpa_demo /dev/ttyACM0" << std::endl;
    std::cerr << "  dpa_demo /dev/spidev0.0" << std::endl;
    return (-1);
  }
  else {
    port_name = argv[1];
  }
  std::cout << "Start demo\n";

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

  DpaLibraryDemo* demo_ = new DpaLibraryDemo(dpaInterface);

  demo_->Start();

  std::cout << "That's all for today...";

  delete demo_;

  return 0;
}

#if 0
/********************************************************************************
* Select type of demo.
********************************************************************************/
//#define SPI_DEMO
#define CDC_DEMO


DpaLibraryDemo* demo_;

#ifdef CDC_DEMO

#include "CdcDpaInterface.h"

int CdcDemoMain(const std::string& port_name);

#define DemoMain CdcDemoMain

#endif /* CDC_DEMO */

#ifdef SPI_DEMO

#include <SpiDpaInterface.h>
#include <sysfs_gpio.h>

int SpiDpaDemoMain();

#define DemoMain SpiDpaDemoMain

#endif /* SPI_DEMO */

int main(int argc, char** argv)
{
  std::string port_name;

  if (argc < 2) {
    std::cerr << "Usage" << std::endl;
    std::cerr << "  dpa_demo <port-name>" << std::endl << std::endl;
    std::cerr << "Example" << std::endl;
    std::cerr << "  cdc_example COM5" << std::endl;
    std::cerr << "  cdc_example /dev/ttyACM0" << std::endl;
    return (-1);
  }
  else {
    port_name = argv[1];
  }
  std::cout << "Start demo\n";
  DemoMain(port_name);
  return 0;
}

/*****************************************************************************
 * CDC DEMO
*****************************************************************************/
#ifdef CDC_DEMO

int CdcDemoMain(const std::string& port_name) {
  CdcDpaInterface* cdcDpaInterface(nullptr);
  try {
    //make a default val here if necessary "/dev/ttyACM0";
    cdcDpaInterface = new CdcDpaInterface(port_name);
  }
  catch (unexpected_peripheral& e) {
    std::cout << e.what() << std::endl;
  }
  //std::cout << cdcDpaInterface->ShowModuleInfo() << std::endl;

  demo_ = new DpaLibraryDemo(cdcDpaInterface);

  demo_->Start();

  std::cout << "That's all for today...";

  delete demo_;

  return 0;
}
#endif


/*****************************************************************************
* SPI DEMO
*****************************************************************************/
#ifdef SPI_DEMO

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
  }
  catch (...) {
    // clean the mess in all cases
  }

  dpaInterface.Close();
  delete demo_;

  // destroy used rpi_io library
  gpio_cleanup(RESET_GPIO);
  gpio_cleanup(RPIIO_PIN_CE0);

}
#endif
#endif //#if 0

DpaLibraryDemo::DpaLibraryDemo(IChannel* communication_interface)
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
    std::cout << "There was an error during DPA handler creation: " << ae.what() << std::endl;
  }

  dpa_handler_->Timeout(100);    // Default timeout is infinite

  int16_t i = 100;
  //wait for a while, there could be some unread message in CDC
  std::this_thread::sleep_for(std::chrono::seconds(1));

  while (i--) {
    //PulseLed(0x00, kLedRed);    // Pulse with red led on coordinator
    //PulseLed(0x00, kLedGreen);    // Pulse with green led on coordinator
    //PulseLed(0x03, kLedRed);    // Pulse with red led on node with address 3
    //PulseLed(0x03, kLedGreen);    // Pulse with green led on node with address 3
    //PulseLed(0xFF, kLedRed);    // Pulse with red led on node with address 3
    //PulseLed(0xFF, kLedGreen);    // Pulse with green led on node with address 3
    //ReadTemperature(0x03);        // Get temperature from node with address 3
    //ReadTemperature(0x00);        // Get temperature from coordinator
    //ReadTemperatureDpaTransaction(0x01); // Get temperature from node1
    //ReadTemperatureDpaTransaction(0x06); // Get temperature from node6
    PulseLedRDpaTransaction(0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  }
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
  message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

  ExecuteCommand(message);
}

void DpaLibraryDemo::ExecuteCommand(DpaMessage& message) {
  static uint16_t sent_messages = 0;
  static uint16_t timeouts = 0;
  try {
    dpa_handler_->SendDpaMessage(message);
  }
  catch (std::logic_error& le) {
    std::cout << "Send error occured: " << le.what() << std:: endl;
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
  static int num(0);
  DpaMessage::DpaPacket_t packet;
  packet.DpaRequestPacket_t.NADR = address;
  packet.DpaRequestPacket_t.PNUM = PNUM_THERMOMETER;

  packet.DpaRequestPacket_t.PCMD = CMD_THERMOMETER_READ;
  packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;

  DpaMessage message;
  message.DataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));

  ExecuteCommand(message);

  if (dpa_handler_->Status() == DpaRequest::DpaRequestStatus::kProcessed) {
    int16_t temperature =
      dpa_handler_->CurrentRequest().ResponseMessage().DpaPacket().DpaResponsePacket_t.DpaMessage.PerThermometerRead_Response.IntegerValue;
    std::cout << num++ << " Temperature: "
//      << std::dec << temperature << " °C\n";
    << std::dec << temperature << " oC" << std::endl;
  }
}

void DpaLibraryDemo::ReadTemperatureDpaTransaction(uint16_t address)
{
  DpaThermometer thermometer(address);
  DpaTransactionTask transaction(thermometer);
  dpa_handler_->ExecuteDpaTransaction(transaction);
  int errorCode = transaction.waitFinish();
  if (errorCode == 0)
    std::cout << NAME_PAR(Temperature, thermometer.getTemperature()) << std::endl;
  else
    std::cout << "Failed to read Temperature at: " << NAME_PAR(addres, thermometer.getAddress()) << PAR(errorCode) << std::endl;
}

void DpaLibraryDemo::PulseLedRDpaTransaction(uint16_t address)
{
  DpaPulseLedR pulse(address);
  DpaTransactionTask transaction(pulse);
  dpa_handler_->ExecuteDpaTransaction(transaction);
  int errorCode = transaction.waitFinish();
  if (errorCode == 0)
    std::cout << pulse << std::endl;
  else
    std::cout << transaction.getErrorStr() << std::endl;
}
