#ifndef DPALIBRARY_SPIDPAINTERFACEIMPL_H
#define DPALIBRARY_SPIDPAINTERFACEIMPL_H

#include <cstdint>
#include <string>
#include <functional>
#include <thread>
#include <mutex>

#include "SpiIqrf.h"

class SpiDpaInterfaceImpl {
 public:
  SpiDpaInterfaceImpl();
  virtual ~SpiDpaInterfaceImpl();

  void Open(std::string device);
  void Close();
  int32_t SendRequest(unsigned char* data, uint32_t length);
  int32_t RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function);
 private:
  SpiIqrf* spiIqrf_;
  SpiIqrfCommunicationStatus lastSpiStatus_;
  std::function<void(unsigned char*, uint32_t)> response_callback_;
  std::thread listenThread_;
  bool isOpen_;
  bool stopCommunication_;
  std::mutex readThreadMutex_;
  std::mutex callbackFunctionMutex_;
  std::mutex writeQueueMutex_;
  uint8_t* writeQueueBuffer_;
  uint32_t writeBufferDataCount_;

  void CommunicationThread();
  bool CheckReadStatus();
  bool CheckWriteQueue() const;
  void ReadDataFromIqrf();
  void WriteDataToIqrf();
  void WriteDataToQueue(void* data, uint32_t length);
  bool CheckEndCommunication() const;
  void CloseCommunication();
};

#endif //DPALIBRARY_SPIDPAINTERFACEIMPL_H
