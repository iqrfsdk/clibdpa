#ifndef DPALIBRARY_SPIIQRF_H
#define DPALIBRARY_SPIIQRF_H

#include <cstdint>
#include <string>
#include "SpiIqrfCommunicationStatus.h"

class SpiIqrf {
public:
    enum SpiIqrfCommunicationSpeedMode{
        kSpiIqrfLowSpeedMode,
        kSpiIqrfHighSpeedMode,
    };

  SpiIqrf();
    virtual ~SpiIqrf();

    void Open(std::string device);
    void Close();
    bool IsOpen();
    void CommunicationMode(SpiIqrfCommunicationSpeedMode mode);
    SpiIqrfCommunicationSpeedMode CommunicationSpeedMode() const;
    SpiIqrfCommunicationStatus CommunicationStatus();
  void Read(void* readBuffer, uint32_t dataLength);
  void Write(void* writeBuffer, uint32_t dataLength);

private:
    bool isOpen_;
};


#endif //DPALIBRARY_SPIIQRF_H
