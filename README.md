# IQRF - DPA Library

[![Build Status](https://travis-ci.org/Roman3349/clibdpa.svg?branch=master)](https://travis-ci.org/Roman3349/clibdpa)

IQRF DPA communication library with basic support of sending and receiving messages in DPA protocol message format. The library is based on DPA.h file where main message structures are defined. Messages are sent via DpaInterface class. DPA Library has no dependency on OS.
The core of library is DpaHandler class. It contains functions for sending and asynchronously receiving data from communication interface. All DPA commands are composed from request and response for coordinator requests and from request, confirmation and response in case of remote node. All states are internally controlled by DpaHandler class.

Demo application uses implementation of DpaInterface for CDC message parser. CDC message parser is available for Linux and Microsoft Windows OS.


Features
* compatible with all DCTR modules
* supported programming languages: C++
* supported operating systems: Linux, Windows


Library contains following folders
```
src 						
	DpaLibrary				Source codes of the IQRF DPA library
		include				Folde contains header files for library
	DpaLibraryDemo 			Example of DPA library implementation
```

## How to use DPA Library
To be able to use DPA Library you will need some communication interface based on abstract class defined in DpaInterface.h file.

### CDC Dpa Interface
For demo purposes we prepared implementation of CDC DPA interface. This interface is used by DPA Library. Source codes are in directory `src/DpaInterfaces/CdcDpaInterface`. Static library with CDC driver from [this repository](https://github.com/iqrfsdk/clibcdc-linux) should be stored in lib/ windows or lib/linux folder depending on your system. Function open has one parameter, the device name. This parameter is communication port identifier. On Linux based OS is in format `"/dev/ttyACM0"` and `"COM1"` on Windows machines.  Do not forget to set right privileges on Linux for port.
```c
CdcDpaInterface cdcDpaInterface;

try {
cdcDpaInterface.Open("/dev/ttyACM0");
}
catch (...) {
return -1;
}
```

### Raspberry Pi SPI Interface
The other interface prepared for demo is communication interface for Raspberry Pi via SPI. Demo is very similar, but you have to create an instance of `SpiDpaInterface`, open the SPI port and use it. Source code of SpiDpaInterface is available in subdirectory src/DpaInterfaces/SpiDpaInterface. Library uses clibspi-linux static library from [this repository](https://github.com/iqrfsdk/clibspi-linux). Compiled library store into lib/windows or lib/linux folder.

```c
SpiDpaInterface communication_interface;
try {
	communication_interface.Open("/dev/spidev0.0");
}
catch (...) {
	// clean and return, something wrong was happened
}
```

If you are using Raspberry Pi with KON-RASP-01, you have to enable power for module and enable `CE0`. At the end, of course, disable and clean these GPIO.
```c
gpio_setup(RPIIO_PIN_CE0, GPIO_DIRECTION_OUT, 0);
gpio_setup(RESET_GPIO, GPIO_DIRECTION_OUT, 1);

// Enable SPI and run demo

gpio_cleanup(RESET_GPIO);
gpio_cleanup(RPIIO_PIN_CE0);

```

Now, you are ready to use DPA Library. In fact, you need only `DpaMessage` and `DpaHandler` classes. DpaMessage holds information about DPA packet used in IQRF DPA environment. All settings and packet types are described in DPA Framework Technical guide.  DpaHandler takes care about your requests, timeouts and other communication related stuffs.
Create an instance of DpaHandler with communication interface and use it. The example bellow pulses with green LED on the coordinator module.
```c
DpaMessage message;
auto dpa_handler_ = DpaHandler(communication_interface);
DpaMessage::DpaPacket_t packet;

packet.DpaRequestPacket_t.NADR = 0x00;
packet.DpaRequestPacket_t.PNUM = PNUM_LEDG;
packet.DpaRequestPacket_t.PCMD = CMD_LED_PULSE;
packet.DpaRequestPacket_t.HWPID = HWPID_DoNotCheck;
message.AddDataToBuffer(packet.Buffer, sizeof(TDpaIFaceHeader));
dpa_handler_.SendDpaMessage(message);
while (dpa_handler_->IsDpaMessageInProgress()) {
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```
One last thing at the end. CDC parser calls function with two parameters when it receives  packet. Use method like below to forward this event to CDC communication interface.
```c
void asyncMsgListener(unsigned char* data, unsigned int length) {
	communication_interface->ReceiveData(data, length);
}
```
## The main library functions
### DPA Handler
|Function|Comments|
|---|---|
|`IsDpaMessageInProgress()`|Queries if DPA message is in progress. Returns true if is in progress, false if not.|
|`SendDpaMessage(DpaMessage& message)`|Sends DPA message to the network, sets DPA handler to process received messages.|
|`DpaRequest& CurrentRequest()`|Gets current request. The request holds information about delivery status and response.|
|`Timeout(int32_t timeout_ms)`|Sets default timeout for DPA message processing.|

### DPA Request
|Function|Comments|
|---|---|
|`DpaMessage& ResponseMessage()`|Response for sent command|.

## Quick command line compilation on the target
```
git clone https://github.com/iqrfsdk/clibdpa.git
cd clibdpa
mkdir -p .build; cd .build; cmake ..; make -j4
```
>To be able to compile demo, copy appropriate CDC library into lib folder.


See [wiki](https://github.com/MICRORISC/iqrfsdk/wiki) for more information.
