#include <stdexcept>
#include "SpiIqrf.h"
#include "spi_iqrf.h"

SpiIqrf::SpiIqrf()
	: isOpen_(isOpen_) {

}

SpiIqrf::~SpiIqrf() {
  if (IsOpen()) {
	Close();
  }
}

void SpiIqrf::Open(std::string device) {
  if (IsOpen()) {
	throw std::logic_error("Communication is already open.");
  }

  auto operResult = spi_iqrf_init(device.c_str());
  if (operResult < 0) {
	throw std::logic_error("Communication interface has not been open.");
  }
  isOpen_ = true;
}

void SpiIqrf::Close() {
  spi_iqrf_destroy();
  isOpen_ = false;
}

bool SpiIqrf::IsOpen() {
  return isOpen_;
}

void SpiIqrf::CommunicationMode(SpiIqrf::SpiIqrfCommunicationSpeedMode mode) {
  _spi_iqrf_CommunicationMode commMode;
  switch (mode) {
	case kSpiIqrfHighSpeedMode:
	  commMode = SPI_IQRF_HIGH_SPEED_MODE;
	  break;
	default:
	  commMode = SPI_IQRF_LOW_SPEED_MODE;
	  break;
  }
  spi_iqrf_setCommunicationMode(commMode);
  //TODO: Mozna pridat moznost kontroly, jestli se zmena provedla
}

SpiIqrf::SpiIqrfCommunicationSpeedMode SpiIqrf::CommunicationSpeedMode() const {
  auto commMode = spi_iqrf_getCommunicationMode();
  if (commMode == SPI_IQRF_HIGH_SPEED_MODE)
	return kSpiIqrfHighSpeedMode;
  return kSpiIqrfLowSpeedMode;
}

void SpiIqrf::Read(void* readBuffer, uint32_t dataLength) {
  if (!IsOpen()) {
	throw std::logic_error("Communicatin interface is not open.");
  }

  if (readBuffer == nullptr) {
	throw std::invalid_argument("readBuffer argument can not be nullptr.");
  }

  if (dataLength <= 0) {
	throw std::invalid_argument("dataLength is bellow or equal zero.");
  }

  if (dataLength > SPI_IQRF_MAX_DATA_LENGTH) {
	throw std::invalid_argument("dataLength exceeds maximal allowed length of data.");
  }

  auto operResult = spi_iqrf_read(readBuffer, dataLength);
  if (operResult < 0) {
	throw std::logic_error("Data has not been writeln.");
  }
}

void SpiIqrf::Write(void* writeBuffer, uint32_t dataLength) {
  if (!IsOpen()) {
	throw std::logic_error("Communication interface is not open.");
  }

  if (dataLength <= 0) {
	throw std::invalid_argument("dataLength is bellow or equal zero.");
  }

  if (dataLength > SPI_IQRF_MAX_DATA_LENGTH) {
	throw std::invalid_argument("dataLength exceeds maximal allowed length of data.");
  }

  auto operResult = spi_iqrf_write(writeBuffer, dataLength);
  if (operResult < 0) {
	throw std::logic_error("Data has not been writeln.");
  }
}

SpiIqrfCommunicationStatus SpiIqrf::CommunicationStatus() {
  SpiIqrfCommunicationStatus::SpiIqrfCommunicationMode mode;
  spi_iqrf_SPIStatus iqrfMode;
  uint32_t dataCount;

  if (!IsOpen()) {
	throw std::logic_error("Communication interface is not open.");
  }

  auto operResult = spi_iqrf_getSPIStatus(&iqrfMode);

  if (operResult < 0) {
	throw std::logic_error("Communication error.");
  }

  if (iqrfMode.isDataReady) {
	mode = SpiIqrfCommunicationStatus::kSpiCommunicationDataReady;
	dataCount = iqrfMode.dataReady;
	return SpiIqrfCommunicationStatus(mode, dataCount);
  }

  switch (iqrfMode.dataNotReadyStatus) {
	case SPI_IQRF_SPI_DISABLED:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationDisabled;
	  break;
	case SPI_IQRF_SPI_SUSPENDED:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationSuspended;
	  break;
	case SPI_IQRF_SPI_BUFF_PROTECT:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationBufferProtect;
	  break;
	case SPI_IQRF_SPI_CRCM_ERR:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationCrcError;
	  break;
	case SPI_IQRF_SPI_READY_COMM:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationStandardMode;
	  break;
	case SPI_IQRF_SPI_READY_PROG:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationProgrammingMode;
	  break;
	case SPI_IQRF_SPI_READY_DEBUG:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationDebugMode;
	  break;
	case SPI_IQRF_SPI_SLOW_MODE:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationSlowMode;
	  break;
	case SPI_IQRF_SPI_HW_ERROR:
	  mode = SpiIqrfCommunicationStatus::kSpiCommunicationHwError;
	  break;
  }
  return SpiIqrfCommunicationStatus(mode);
}

















