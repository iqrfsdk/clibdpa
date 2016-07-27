#include "SpiDpaInterfaceImpl.h"

SpiDpaInterfaceImpl::SpiDpaInterfaceImpl()
	: isOpen_(false), writeQueueBuffer_(nullptr), writeBufferDataCount_(0) {
  spiIqrf_ = new SpiIqrf();
}

SpiDpaInterfaceImpl::~SpiDpaInterfaceImpl() {
  Close();
  delete spiIqrf_;
  delete writeQueueBuffer_;
}

void SpiDpaInterfaceImpl::Open(std::string device) {
  stopCommunication_ = false;
  spiIqrf_->Open(device);
  listenThread_ = std::thread(&SpiDpaInterfaceImpl::CommunicationThread, this);
  isOpen_ = true;
}

int32_t SpiDpaInterfaceImpl::SendRequest(unsigned char* data, uint32_t length) {
  WriteDataToQueue(data, length);
  return 0;
}

int32_t SpiDpaInterfaceImpl::RegisterResponseHandler(std::function<void(unsigned char*, uint32_t)> function) {
  std::lock_guard<std::mutex> lock(callbackFunctionMutex_);
  response_callback_ = function;
  return 0;
}

void SpiDpaInterfaceImpl::Close() {
  if (!isOpen_) {
	return;
  }

  isOpen_ = false;

  CloseCommunication();
  if (listenThread_.joinable()) {
	listenThread_.join();
  }
}

void SpiDpaInterfaceImpl::CommunicationThread() {
  while (1) {

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	if (CheckEndCommunication()) {
	  break;
	}

	if (CheckReadStatus()) {
	  ReadDataFromIqrf();
	  continue;
	}

	if (lastSpiStatus_.Mode() != SpiIqrfCommunicationStatus::kSpiCommunicationStandardMode) {
	  continue;
	}

	if (CheckWriteQueue()) {
	  WriteDataToIqrf();
	  continue;
	}

  }
}

bool SpiDpaInterfaceImpl::CheckReadStatus() {
  lastSpiStatus_ = spiIqrf_->CommunicationStatus();
  return lastSpiStatus_.IsDataReady();
}

bool SpiDpaInterfaceImpl::CheckWriteQueue() const {
  return (writeBufferDataCount_ > 0);
}

void SpiDpaInterfaceImpl::ReadDataFromIqrf() {
  auto dataCount = lastSpiStatus_.DataCount();
  unsigned char receiveBuffer[dataCount];
  spiIqrf_->Read(receiveBuffer, dataCount);

  std::lock_guard<std::mutex> lock(callbackFunctionMutex_);        //TODO:asi se da vyresit jinak
  auto callback = response_callback_;
  if (callback) {
	callback(receiveBuffer, dataCount);
  }
}

bool SpiDpaInterfaceImpl::CheckEndCommunication() const {
  return stopCommunication_;
}

void SpiDpaInterfaceImpl::CloseCommunication() {
  stopCommunication_ = true;
}
void SpiDpaInterfaceImpl::WriteDataToIqrf() {
  std::lock_guard<std::mutex> lock(writeQueueMutex_);
  spiIqrf_->Write(writeQueueBuffer_, writeBufferDataCount_);
  writeBufferDataCount_ = 0;
}
void SpiDpaInterfaceImpl::WriteDataToQueue(void* data, uint32_t length) {
  std::lock_guard<std::mutex> lock(writeQueueMutex_);
  delete writeQueueBuffer_;
  writeBufferDataCount_ = length;
  writeQueueBuffer_ = new uint8_t[writeBufferDataCount_];
  std::copy((uint8_t*) data, (uint8_t*) data + length * sizeof(uint8_t), writeQueueBuffer_);
}
