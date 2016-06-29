#ifndef __DPA_REQUEST
#define __DPA_REQUEST

#include "DpaMessage.h"
#include <chrono>
#include <mutex>

class DpaRequest {
 public:
  enum DpaRequestStatus {
	kCreated,
	kSent,
	kConfirmation,
	kTimeout,
	kProcessed
  };

  DpaRequest();

  ~DpaRequest();

  void ProcessSentMessage(const DpaMessage& sent_message);

  void ProcessReceivedMessage(const DpaMessage& received_message);

  DpaMessage& SentMessage() {
	return *sent_message_;
  }

  const DpaMessage& ResponseMessage() const {
	return *response_message_;
  }

  void CheckTimeout();

  DpaRequestStatus Status();

  bool IsInProgress();

  int32_t DefaultTimeout() const {
	return timeout_ms_;
  }

  void SetDefaultTimeout(int32_t timeout_ms) {
	this->timeout_ms_ = timeout_ms;
  }

  static int32_t EstimatedTimeout(const DpaMessage& confirmation_packet);

 private:
  DpaRequestStatus status_;
  DpaMessage* sent_message_;
  DpaMessage* response_message_;
  std::mutex status_mutex_;
  std::chrono::system_clock::time_point start_time_;
  int32_t expected_duration_ms_;
  int32_t timeout_ms_;

  void SetTimeoutForCurrentRequest(int32_t time_in_ms);

  bool IsTimeout() const;

  void ProcessConfirmationMessage(const DpaMessage& confirmation_message);

  void ProcessResponseMessage(const DpaMessage& response_message);

  void SetStatus(DpaRequestStatus status);

  static bool IsInProgressStatus(DpaRequestStatus status);

  const int32_t kDefaultTimeout = 200;
};

#endif // !__DPA_REQUEST