#ifndef __DPA_REQUEST
#define __DPA_REQUEST

#include "DpaMessage.h"
#include <chrono>
#include <mutex>

class DpaRequest {
 public:
  /** Values that represent DPA request status. */
  enum DpaRequestStatus {
    ///< An enum constant representing the request is created.
        kCreated,
    ///< An enum constant representing the first message was sent.
        kSent,
    ///< An enum constant representing the confirmation was received.
        kConfirmation,
    kConfirmationBroadcast,
    ///< An enum constant representing the timeout expired.
        kTimeout,
    ///< An enum constant representing the whole request was processed.
        kProcessed
  };

  /** Default constructor. */
  DpaRequest();

  /** Destructor. */
  ~DpaRequest();

  /**
   Processes message sent to network, stores it for future uses and sets status. Only one message per
   request could be sent.

   @exception	std::logic_error	Raised when status is not kCreated.

   @param	sent_message	The message which was sent.
   */
  void ProcessSentMessage(const DpaMessage& sent_message);

  /**
   Processes message received from the network.

   @exception	unexpected_packet_type	Thrown when a message is not a confirmation or response type.
   @exception	unexpected_peripheral 	Thrown when a message is not from the same peripheral like sent message.
   @exception	unexpected_command	  	Thrown when a message is not the response for command from sent message.

   @param	received_message	Received message.
   */
  void ProcessReceivedMessage(const DpaMessage& received_message);

  /**
   Gets a sent message. 

   @return	A reference to the sent message. Otherwise returns nullptr if no sent message was processed.
   */
  const DpaMessage& SentMessage() const {
    return *sent_message_;
  }

  /**
   Gets a response message.

   @return	A reference to the received message. Otherwise returns nullptr if no sent message was processed.
   */
  const DpaMessage& ResponseMessage() const {
    return *response_message_;
  }

  /**
   Gets the status of the request.

   @return	The status of the request.
   */
  DpaRequestStatus Status();

  /**
   Query if request is in progress state.

   @return	true if is in progress, false if not.
   */
  bool IsInProgress();

  /**
   Gets timeout in ms.

   @return	Timeout in ms.
   */
  int32_t DefaultTimeout() const {
    return timeout_ms_;
  }

  /**
   Sets default timeout.

   @param	timeout_ms	Timeout in milliseconds.
   */
  void DefaultTimeout(int32_t timeout_ms) {
    this->timeout_ms_ = timeout_ms;
  }

  /**
   Estimated timeout calculated for confirmation message.

   @param	confirmation_packet	The confirmation DPA message.

   @return	Estimated timeout in ms.
   */
  static int32_t EstimatedTimeout(const DpaMessage& confirmation_packet);

 private:
  DpaRequestStatus status_;
  DpaMessage* sent_message_;
  DpaMessage* response_message_;
  std::mutex status_mutex_;
  std::chrono::system_clock::time_point start_time_;
  int32_t expected_duration_ms_;
  int32_t timeout_ms_;

  void SetTimeoutForCurrentRequest(int32_t extra_time_in_ms = 0);
  bool IsTimeout() const;
  void ProcessConfirmationMessage(const DpaMessage& confirmation_message);
  void ProcessResponseMessage(const DpaMessage& response_message);
  void SetStatus(DpaRequestStatus status);
  void CheckTimeout();
  static bool IsInProgressStatus(DpaRequestStatus status);
};

#endif // !__DPA_REQUEST