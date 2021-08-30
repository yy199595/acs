#include"SceneActionComponent.h"
#include"SceneSessionComponent.h"
#include<Util/StringHelper.h>
#include<Core/App.h>
#include<Util/NumberHelper.h>
#include<NetWork/NetWorkRetAction.h>
#include<Timer/ActionTimeoutTimer.h>

namespace Sentry
{
    SceneActionComponent::SceneActionComponent()
    {
        this->mMessageTimeout = 0;
    }

    bool SceneActionComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		this->mTimerComponent = App::Get().GetTimerComponent();
		SayNoAssertRetFalse_F(config.GetValue("NetWork", "MsgTimeout", this->mMessageTimeout));
        return true;
    }

	unsigned int SceneActionComponent::AddCallback(shared_ptr<LocalRetActionProxy> rpcAction)
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

	bool SceneActionComponent::InvokeCallback(PacketMapper *messageData)
	{
		unsigned int rpcId = messageData->GetRpcId();
		auto iter = this->mRetActionMap.find(rpcId);
		if (iter == this->mRetActionMap.end())
		{
			return false;
		}
		shared_ptr<LocalRetActionProxy> action = iter->second;
		if (action != nullptr)
		{
			action->Invoke(messageData);
#ifdef SOEASY_DEBUG
			const ProtocolConfig *config = messageData->GetProConfig();
			double t = (TimeHelper::GetMilTimestamp() - action->GetCreateTime()) / 1000.0f;
			SayNoDebugWarning(
				"call " << config->ServiceName << "." << config->MethodName << " response" << " [" << t
				<< "s]");
#endif
		}
		this->mNumberPool.Push(rpcId);
		this->mRetActionMap.erase(iter);
		return true;

	}
}
