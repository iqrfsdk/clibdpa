#ifndef DPALIBRARY_DPALIBRARYDEMO_H
#define DPALIBRARY_DPALIBRARYDEMO_H

#include "IChannel.h"
#include "DpaTransactionTask.h"

class DpaTransactionDemo : public DpaTransactionTask
{
public:
  DpaTransactionDemo(DpaTask& dpaTask);
  virtual ~DpaTransactionDemo();
  virtual void processConfirmationMessage(const DpaMessage& confirmation);
  virtual void processResponseMessage(const DpaMessage& response);
};

class DpaLibraryDemo {
public:

  enum LedColor {
    kLedRed,
    kLedGreen
  };


  DpaLibraryDemo(IChannel* communication_interface);

  virtual ~DpaLibraryDemo();

  void Start();

  void ListenerWrapper(unsigned char* data, unsigned int length);

private:
  IChannel* dpaInterface_;
  DpaHandler* dpa_handler_;

  void PulseLed(uint16_t address, LedColor color);

  void ExecuteCommand(DpaMessage& message);

  void UnexpectedMessage(const DpaMessage& message);

  void ReadTemperature(uint16_t address);

  void ReadTemperatureDpaTransaction(uint16_t address);

};


#endif //DPALIBRARY_DPALIBRARYDEMO_H
