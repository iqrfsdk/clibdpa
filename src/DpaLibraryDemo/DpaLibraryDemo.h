#ifndef DPALIBRARY_DPALIBRARYDEMO_H
#define DPALIBRARY_DPALIBRARYDEMO_H

#include "DpaInterface.h"
#include <stdint.h>

class DpaLibraryDemo {
public:

    enum LedColor {
        kLedRed,
        kLedGreen
    };


    DpaLibraryDemo(DpaInterface *communication_interface);

    virtual ~DpaLibraryDemo();

  void Start();

    void ListenerWrapper(unsigned char *data, unsigned int length);

private:
    DpaInterface *dpaInterface_;
    DpaHandler *dpa_handler_;

    void PulseLed(uint16_t address, LedColor color);

    void ExecuteCommand(DpaMessage &message);
};


#endif //DPALIBRARY_DPALIBRARYDEMO_H
