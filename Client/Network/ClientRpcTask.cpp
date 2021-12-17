#include"ClientRpcTask.h"
#include"Core/App.h"
#include<Pool/MessagePool.h>
#include"Util/TimeHelper.h"
#include"ClientComponent.h"
namespace Client
{
	ClientRpcTask::ClientRpcTask(const std::string & method, long long rpcId)
		: mMethod(method), mRpcId(rpcId)
	{
		this->mTimerId = 0;
		this->mTimeout = 0;
		this->mMessage = nullptr;
		this->mStartTime = Helper::Time::GetMilTimestamp();
	}

	void ClientRpcTask::OnResponse(const c2s::Rpc_Response * response)
	{
		if (response == nullptr)
		{
			this->mTaskState = TaskTimeout;
			this->mCode = XCode::CallTimeout;
			LOG_INFO("call " << this->mMethod << " call time out");
		}
		else
		{
			this->mCode = (XCode)response->code();
			if (this->mCode == XCode::Successful && response->has_data())
			{
				auto message = MessagePool::NewByData(response->data(), true);
				this->mMessage = std::shared_ptr<Message>(message);
			}
			long long t1 = Helper::Time::GetMilTimestamp();
			float second = (t1 - this->mStartTime) / 1000.0f;
			LOG_INFO("call " << this->mMethod << " successful time = " << second << "s");
		}
		this->RestoreAsyncTask();
	}

	XCode ClientRpcTask::AwaitGetCode(int ms)
	{
		this->mTimeout = ms;
        this->AwaitTask();
		return this->mCode;
	}
	void ClientRpcTask::OnTaskAwait()
	{
		auto clientComponent = App::Get().GetComponent<ClientComponent>();
		this->mTimerId = clientComponent->AddRpcTask(this->shared_from_this(), this->mTimeout);
	}
}
