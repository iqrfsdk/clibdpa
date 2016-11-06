#ifndef __DPA_HANDLER
#define __DPA_HANDLER

#include "DpaMessage.h"
#include "DpaRequest.h"
#include "IChannel.h"
#include "DpaTransaction.h"
#include <memory>
#include <queue>
#include <functional>
#include <cstdint>
#include <mutex>
#include <condition_variable>

class DpaHandler {
 public:
  /**
   Constructor.
  
   @param [in,out]	dpa_interface	Pointer to instance of DPA interface.
   */
  DpaHandler(IChannel* dpa_interface);

  /** Destructor. */
  ~DpaHandler();

  /**
   Query if DPA message is in progress.
  
   @return	true if DPA message is in progress, false if not.
   */
  bool IsDpaMessageInProgress() const;

  /**
  Query if DPA Transaction is in progress.

  @param [in,out]	expected_duration	Expected time to finish DPA transaction.
  @return	true if DPA message is in progress, false if not.
  */
  bool IsDpaTransactionInProgress(int32_t& expected_duration) const;

  /**
   Gets the status.
  
   @return	Status of current DPA request.
   */
  DpaRequest::DpaRequestStatus Status() const;

  /**
   Handler, called when the response.
  
   @param [in,out]	data	Pointer to received data.
   @param	length			Length of received bytes.
   */
  void ResponseHandler(const std::basic_string<unsigned char>& message);

  /**
   Sends a DPA message.
  
   @param [in,out]	DPA message to be send.
   */
  void SendDpaMessage(const DpaMessage& message, DpaTransaction* responseHndl = nullptr);
  
  /**
  Executes a DPA transaction.
  The method blocks until received response or timeout

  @param [in,out]	DPA transaction to be executed.
  */
  void ExecuteDpaTransaction(DpaTransaction& dpaTransaction);

  /**
   Registers the function called when unexpected message is received.
  
   @param	Pointer to called function.
   */
  void RegisterAsyncMessageHandler(std::function<void(const DpaMessage&)> message_handler);

  /** Unregister function for unexpected messages. */
  void UnregisterAsyncMessageHandler();

  /**
   Returns reference to currently processed request.

   @return Current request.
   */
  const DpaRequest& CurrentRequest() const;

  /**
   Sets timeout for all new requests.

   @param Timeout in ms.
   */
  void Timeout(int32_t timeout_ms);

  /*
   Gets value of timeout in ms.

   @return Timeout in ms;
   */
  int32_t Timeout() const;

 private:
  /** The current request. */
  DpaRequest* current_request_;
  /** The DPA communication interface. */
  IChannel* dpa_interface_;
  /** Holds the pointer to function called when unexpected message is received. */
  std::function<void(const DpaMessage&)> async_message_handler_;
  /** Lock used for safety using of async_message_handler. */
  std::mutex async_message_mutex_;

  /** Lock used in transaction handling */
  std::mutex condition_variable_mutex_;
  /** Condition to wait in transaction handling */
  std::condition_variable condition_variable_;

  /**
   Process received message.
  
   @param	The message to be processed.
  
   @return	true if succeeds, false if fails.
   */
  bool ProcessMessage(const DpaMessage& message);

  /**
   Process the unexpected message described by message.
  
   @param [in,out]	message	The message.
   */
  void ProcessUnexpectedMessage(DpaMessage& message);

  /** Holds timeout in ms. */
  int32_t default_timeout_ms_;

  /** Default value of timeout in ms.*/
  const int32_t kDefaultTimeout = -1;
};

#endif // !__DPA_HANDLER
