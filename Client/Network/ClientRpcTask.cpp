#include"ClientRpcTask.h"
#include"Object/App.h"
#include<Pool/MessagePool.h>
#include"Util/TimeHelper.h"
#include"ClientComponent.h"
#include"Coroutine/TaskComponent.h"
namespace Client
{
	ClientRpcTask::ClientRpcTask(const std::string & method)
		: mMethod(method)
    {
        this->mTimerId = 0;
        this->mTimeout = 0;
        this->mCoroutineId = 0;
        this->mTaskComponent = nullptr;
        this->mClientComponent = nullptr;
        this->mState = TaskState::TaskReady;
        this->mRpcId = Helper::Guid::Create();
        this->mStartTime = Helper::Time::GetMilTimestamp();
        this->mTaskComponent = App::Get().GetTaskComponent();
    }

	void ClientRpcTask::OnResponse(const c2s::Rpc_Response * response)
	{
        LOG_CHECK_RET(this->mState == TaskState::TaskAwait);
		if (response == nullptr)
		{
			this->mCode = XCode::CallTimeout;
			LOG_INFO("call " << this->mMethod << " call time out");
		}
		else
		{
			this->mCode = (XCode)response->code();
			if (this->mCode == XCode::Successful && response->has_data())
			{
				auto message = Helper::Proto::NewByData(response->data(), true);
				this->mMessage = std::shared_ptr<Message>(message);
			}
			long long t1 = Helper::Time::GetMilTimestamp();
			float second = (t1 - this->mStartTime) / 1000.0f;
			LOG_INFO("call " << this->mMethod << " successful time = " << second << "s");
		}
        this->mState = TaskState::TaskFinish;
        this->mTaskComponent->Resume(this->mCoroutineId);
	}

    bool ClientRpcTask::YieldTask()
    {
        if(this->mState == TaskState::TaskReady)
        {
            this->mState = TaskState::TaskAwait;
            this->mTaskComponent = App::Get().GetComponent<TaskComponent>();
            this->mClientComponent = App::Get().GetComponent<ClientComponent>();
            this->mTimerId = this->mClientComponent->AddRpcTask(this->shared_from_this(), 5000);
            this->mTaskComponent->Yield(this->mCoroutineId);
            return true;
        }
        return false;
    }

    XCode ClientRpcTask::GetCode()
    {
        this->YieldTask();
        return this->mCode;
    }
}
