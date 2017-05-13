/**
 * Copyright 2015-2017 MICRORISC s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

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
	 enum IqrfRfCommunicationMode
	 {
		 kStd,
		 kLp
	 };

	 enum DpaProtocolVersion
	{
		k22x,
		k30x
	};

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

	DpaRequest* CreateDpaRequest(DpaTransaction* dpa_transaction) const;
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

  DpaProtocolVersion DpaVersion() const;
  void DpaVersion(DpaProtocolVersion new_dpa_version);

	IqrfRfCommunicationMode GetRfCommunicationMode() const;

  void SetRfCommunicationMode(IqrfRfCommunicationMode mode);

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

  DpaProtocolVersion current_dpa_version_;
  IqrfRfCommunicationMode current_communication_mode_;
};
