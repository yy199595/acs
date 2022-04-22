//
// Created by yjz on 2022/4/17.
//

#include "RequestTaskQueueSource.h"

namespace Sentry
{
	std::shared_ptr<c2s::Rpc::Request> RequestTaskQueueSource::Await()
	{
		this->YieldTask();
		if(!this->mTaskQueue.empty())
		{
			mRequest = this->mTaskQueue.front();
			this->mTaskQueue.pop();
		}
		return mRequest;
	}

	void RequestTaskQueueSource::AddResult(std::shared_ptr<c2s::Rpc::Request>& data)
	{
		this->mTaskQueue.push(data);
		this->ResumeTask(TaskState::TaskReady);
	}
}