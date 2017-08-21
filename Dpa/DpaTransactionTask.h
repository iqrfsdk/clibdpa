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

#include <future>

#include "DpaTransaction.h"
#include "DpaTask.h"

// implements transaction
class DpaTransactionTask : public DpaTransaction
{
public:
	DpaTransactionTask() = delete;
	DpaTransactionTask(DpaTask& dpaTask);
	DpaTransactionTask(const DpaTransactionTask&) = delete;
	DpaTransactionTask(DpaTransactionTask&&) = delete;
	DpaTransactionTask& operator= (const DpaTransactionTask&) = delete;
	DpaTransactionTask& operator= (DpaTransactionTask&&) = delete;

	virtual ~DpaTransactionTask();

	// interface
	const DpaMessage& getMessage() const override;
	int getTimeout() const override;

	void processConfirmationMessage(const DpaMessage& confirmation) override;
	void processResponseMessage(const DpaMessage& response) override;
	
	// set promise object according to return state of dpa transfer
	void processFinish(DpaTransfer::DpaTransferStatus status) override;

	//0: success, -1: DpaHandler timeout, -2: future timeout, <n>: responseCode
	int waitFinish();

	// errors
	int getError() const;
	std::string getErrorStr() const;

private:
	DpaTask& m_dpaTask;

	//The promise object is the asynchronous provider and is expected to set a value for the shared state at some point.
	//The future object is an asynchronous return object that can retrieve the value of the shared state, waiting for it to be ready, if necessary.
	std::promise<int> m_promise;
	std::future<int> m_future;
	
	int m_error;
};
