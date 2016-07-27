#ifndef DPALIBRARY_SPIIQRFCOMMUNICATIONSTATUS_H
#define DPALIBRARY_SPIIQRFCOMMUNICATIONSTATUS_H

#include <cstdint>

class SpiIqrfCommunicationStatus {
 public:
  enum SpiIqrfCommunicationMode {
	kSpiCommunicationDisabled,
	kSpiCommunicationSuspended,
	kSpiCommunicationBufferProtect,
	kSpiCommunicationCrcError,
	kSpiCommunicationStandardMode,
	kSpiCommunicationProgrammingMode,
	kSpiCommunicationDebugMode,
	kSpiCommunicationSlowMode,
	kSpiCommunicationHwError,
	kSpiCommunicationDataReady
  };
  SpiIqrfCommunicationStatus();
  SpiIqrfCommunicationStatus(const SpiIqrfCommunicationStatus& other);
  explicit SpiIqrfCommunicationStatus(SpiIqrfCommunicationMode status, uint32_t dataCount = 0);

  SpiIqrfCommunicationStatus& operator==(const SpiIqrfCommunicationStatus& other);

  bool IsDataReady() const;
  uint32_t DataCount() const;
  SpiIqrfCommunicationMode Mode() const;
 private:
  SpiIqrfCommunicationMode iqrfMode_;
  uint32_t dataCount_;
};

#endif //DPALIBRARY_SPIIQRFCOMMUNICATIONSTATUS_H
