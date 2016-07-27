#include "SpiIqrfCommunicationStatus.h"

bool SpiIqrfCommunicationStatus::IsDataReady() const {
    return (dataCount_ > 0);
}

uint32_t SpiIqrfCommunicationStatus::DataCount() const {
    return dataCount_;
}

SpiIqrfCommunicationStatus::SpiIqrfCommunicationStatus(SpiIqrfCommunicationMode status, uint32_t dataCount)
    : iqrfMode_(status)
    , dataCount_(dataCount)
{

}

SpiIqrfCommunicationStatus::SpiIqrfCommunicationStatus(const SpiIqrfCommunicationStatus &other)
    : iqrfMode_(other.iqrfMode_)
    , dataCount_(other.dataCount_)
{

}

SpiIqrfCommunicationStatus &SpiIqrfCommunicationStatus::operator==(const SpiIqrfCommunicationStatus &other) {
    if (this == &other)
        return *this;

    iqrfMode_ = other.iqrfMode_;
    dataCount_ = other.dataCount_;

    return *this;
}

SpiIqrfCommunicationStatus::SpiIqrfCommunicationStatus() {
  iqrfMode_ = kSpiCommunicationHwError;
  dataCount_ = 0;
}
SpiIqrfCommunicationStatus::SpiIqrfCommunicationMode SpiIqrfCommunicationStatus::Mode() const {
  return iqrfMode_;
}





