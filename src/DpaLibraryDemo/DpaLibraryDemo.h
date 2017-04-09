#ifndef DPALIBRARY_DPALIBRARYDEMO_H
#define DPALIBRARY_DPALIBRARYDEMO_H

#include "IChannel.h"
#include "DpaTransactionTask.h"

class DpaLibraryDemo {
public:

  enum LedColor {
    kLedRed,
    kLedGreen
  };


  DpaLibraryDemo(IChannel* communication_interface);

  virtual ~DpaLibraryDemo();

  void Start();
  void CommunicationTest();

  void ListenerWrapper(unsigned char* data, unsigned int length);

private:
  IChannel* dpaInterface_;
  DpaHandler* dpa_handler_;

  void PulseLed(uint16_t address, LedColor color);

  void ExecuteCommand(DpaMessage& message);

  void UnexpectedMessage(const DpaMessage& message);

  void ReadTemperature(uint16_t address);

  void ReadTemperatureDpaTransaction(uint16_t address);

  void PulseLedRDpaTransaction(uint16_t address);
  void ReadFrcReadTemperature();

  void OpenUart(uint16_t address, uint8_t br);
  void SendDataToUart(uint16_t address, uint8_t timeout, uint8_t* data, uint8_t dataCount);
  uint16_t CoordinatorGetAddressingInfo();

};


#endif //DPALIBRARY_DPALIBRARYDEMO_H
