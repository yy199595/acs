#include"ActionComponent.h"
#include"NetSessionComponent.h"
#include<Util/StringHelper.h>
#include<Core/App.h>
#include<Util/NumberHelper.h>
#include<NetWork/NetWorkRetAction.h>
#include<Timer/ActionTimeoutTimer.h>

namespace Sentry
{
    ActionComponent::ActionComponent()
    {
        this->mMessageTimeout = 0;
    }

    bool ActionComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		this->mTimerComponent = App::Get().GetTimerComponent();
		SayNoAssertRetFalse_F(config.GetValue("NetWork", "MsgTimeout", this->mMessageTimeout));
        return true;
    }

	unsigned int ActionComponent::AddCallback(shared_ptr<LocalRetActionProxy> rpcAction)
    {
        if (rpcAction == nullptr)
        {
            return 0;
        }
		unsigned int id = this->mNumberPool.Pop();     
        if (this->mMessageTimeout != 0)// 添加超时
        {
            shared_ptr<ActionTimeoutTimer> timer = 
				make_shared<ActionTimeoutTimer>(this->mMessageTimeout, id,                                                                                 this);
            this->mTimerComponent->AddTimer(timer);
        }
        this->mRetActionMap.emplace(id, rpcAction);
        return id;
    }

	bool ActionComponent::InvokeCallback(PacketMapper *messageData)
	{
		unsigned int rpcId = messageData->GetRpcId();
		auto iter = this->mRetActionMap.find(rpcId);
		if (iter == this->mRetActionMap.end())
		{
			return false;
		}
		iter->second->Invoke(messageData);
#ifdef _DEBUG
		memset(this->mBuffer, 0, 100);
		const ProtocolConfig *config = messageData->GetProConfig();
		float t = (TimeHelper::GetMilTimestamp() - iter->second->GetCreateTime()) / 1000.0f;
		SayNoDebugLog("call " << config->ServiceName << "." << config->MethodName << "  response [elapsed time = ]" << t);
#endif

		this->mNumberPool.Push(rpcId);
		this->mRetActionMap.erase(iter);
		return true;
	}
}
