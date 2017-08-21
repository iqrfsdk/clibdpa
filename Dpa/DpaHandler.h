/**
 * Copyright 2015-2017 MICRORISC s.r.o.
 * Copyright 2017 IQRF Tech s.r.o.
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

#include <memory>
#include <queue>
#include <functional>
#include <cstdint>
#include <mutex>
#include <condition_variable>

#include "DpaTransfer.h"
#include "DpaTransaction.h"
#include "DpaMessage.h"
#include "IChannel.h"

class DpaHandler {
public:
	/** Values that represent IQRF communication modes. */
	enum IqrfRfCommunicationMode {
		kStd,
		kLp
	};

	/**
	 Constructor.

	 @param [in,out]	iqrfInterface	Pointer to instance of IQRF interface.
	 */
	DpaHandler(IChannel* iqrfInterface);

	/** Destructor. */
	~DpaHandler();

	/**
	Sends a DPA message.

	@param [in,out]	DPA message to be send.
	*/
	void SendDpaMessage(const DpaMessage& message, DpaTransaction* responseHandler = nullptr);

	/**
	Handler, called when the response.

	@param [in,out]	data	Pointer to received data.
	@param	length			Length of received bytes.
	*/
	void ResponseMessageHandler(const std::basic_string<unsigned char>& message);
	DpaTransfer* CreateDpaTransfer(DpaTransaction* dpaTransaction) const;

	/**
	Returns reference to currently running DPA transfer.

	@return Current transfer.
	*/
	const DpaTransfer& CurrentTransfer() const;

	/**
	 Query if DPA message is in progress.

	 @return	true if DPA message is in progress, false if not.
	 */
	bool IsDpaMessageInProgress() const;

	/**
	Query if DPA Transaction is in progress.

	@param [in,out]	expectedDuration	Expected time to finish DPA transaction.
	@return	true if DPA message is in progress, false if not.
	*/
	bool IsDpaTransactionInProgress(int32_t& expectedDuration) const;

	/**
	 Gets the status.

	 @return	Status of current DPA session.
	 */
	DpaTransfer::DpaTransferStatus Status() const;
	
	/**
	 Registers the function called when asynchronous message is received.

	 @param	Pointer to called function.
	 */
	void RegisterAsyncMessageHandler(std::function<void(const DpaMessage&)> messageHandler);

	/** Unregister function for unexpected messages. */
	void UnregisterAsyncMessageHandler();

	/**
	 Sets timeout for all new requests.

	 @param Timeout in ms.
	 */
	void Timeout(int32_t timeoutMs);

	/*
	 Gets value of timeout in ms.

	 @return Timeout in ms;
	 */
	int32_t Timeout() const;

	/**
	Set and Get IQRF communication mode.

	@param	IqrfRfCommunicationMode	mode.
	@return	IqrfRfCommunicationMode	mode.
	*/
	void SetRfCommunicationMode(IqrfRfCommunicationMode mode);
	IqrfRfCommunicationMode GetRfCommunicationMode() const;

	/**
	Executes a DPA transaction.
	The method blocks until received response or timeout

	@param [in,out]	DPA transaction to be executed.
	*/
	void ExecuteDpaTransaction(DpaTransaction& dpaTransaction);
	void KillDpaTransaction();

private:
	/** Default value of timeout in ms.*/
	const int32_t kDefaultTimeout = -1;
	/** Holds timeout in ms. */
	int32_t m_defaultTimeoutMs;
	/** Holds the communication mode */
	IqrfRfCommunicationMode m_currentCommunicationMode;

	/** The current transfer. */
	DpaTransfer* m_currentTransfer;
	/** The IQRF communication interface. */
	IChannel* m_iqrfInterface;
	
	/** Holds the pointer to function called when asynchronous message is received. */
	std::function<void(const DpaMessage&)> m_asyncMessageHandler;
	/** Lock used for safety using of async_message_handler. */
	std::mutex m_asyncMessageMutex;

	/** Lock used in transaction handling */
	std::mutex m_conditionVariableMutex;
	/** Condition to wait in transaction handling */
	std::condition_variable m_conditionVariable;

	/**
	 Process received message.

	 @param	The message to be processed.

	 @return	true if succeeds, false if fails.
	 */
	bool ProcessMessage(const DpaMessage& message);

	/**
	 Process the asynchronous message described by message.

	 @param [in,out]	message	the message.
	 */
	void ProcessAsynchronousMessage(DpaMessage& message);
};
