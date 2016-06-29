#ifndef __DPA_HANDLER
#define __DPA_HANDLER

#include <memory>
#include "DpaMessage.h"
#include <queue>
#include <functional>
#include "DpaRequest.h"
#include "DpaInterface.h"
#include <cstdint>

class DpaHandler {
 public:
  DpaHandler(DpaInterface* dpa_interface);

  ~DpaHandler();

  bool IsDpaMessageInProgress() const;

  DpaRequest::DpaRequestStatus Status() const;

  void ResponseHandler(unsigned char* data, uint32_t length);

  void SendDpaMessage(DpaMessage& message);
  void RegisterAsyncMessageHandler(std::function<void(const DpaMessage&)> message_handler);
  void UnregisterAsyncMessageHandler();

 private:
  DpaRequest* current_request_;
  DpaInterface* dpa_interface_;
  std::function<void(const DpaMessage&)> async_message_handler_;
  std::mutex async_message_mutex_;

  bool ProcessMessage(const DpaMessage& message);
  void ProcessUnexpectedMessage(DpaMessage& message);
};

#endif // !__DPA_HANDLER
