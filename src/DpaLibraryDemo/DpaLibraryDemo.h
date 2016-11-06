#ifndef DPALIBRARY_DPALIBRARYDEMO_H
#define DPALIBRARY_DPALIBRARYDEMO_H

#include "IChannel.h"
#include "DpaTransaction.h"

class DpaTransactionDemo : public DpaTransaction
{
public:
  DpaTransactionDemo(DpaTask& dpaTask);
  virtual ~DpaTransactionDemo();
  virtual const DpaMessage& getMessage() const { return m_dpaTask.getRequest(); }
  virtual void processConfirmationMessage(const DpaMessage& confirmation);
  virtual void processResponseMessage(const DpaMessage& response);
  virtual void processFinished(bool success);
  
  bool isSuccess() { return m_success; }
private:
  DpaTask& m_dpaTask;
  bool m_success;
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
