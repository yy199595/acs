#include"CallHandlerComponent.h"
#include<Util/StringHelper.h>
#include<Core/App.h>
#include<Util/NumberHelper.h>
#include<Method/CallHandler.h>
#include<Timer/ActionTimeoutTimer.h>

#ifdef __DEBUG__
#include <Scene/ProtocolComponent.h>
#endif
namespace Sentry
{
    CallHandlerComponent::CallHandlerComponent()
        : mNumberPool(100)
    {
        this->mMessageTimeout = 0;
    }

    bool CallHandlerComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		this->mTimerComponent = App::Get().GetTimerComponent();
		SayNoAssertRetFalse_F(config.GetValue("NetWork", "MsgTimeout", this->mMessageTimeout));
        return true;
    }

	bool CallHandlerComponent::AddCallHandler(CallHandler * rpcAction, unsigned int & id)
    {
        if (rpcAction == nullptr)
        {
            return false;
        }
		id = this->mNumberPool.Pop();
        if (this->mMessageTimeout != 0)// 添加超时
        {
            shared_ptr<ActionTimeoutTimer> timer = 
				make_shared<ActionTimeoutTimer>(this->mMessageTimeout, id,                                                                                 this);
            this->mTimerComponent->AddTimer(timer);
        }
        this->mRetActionMap.emplace(id, rpcAction);
        return true;
    }

    bool CallHandlerComponent::OnResponseMessage(const com::DataPacket_Response & response)
    {
        unsigned int rpcId = response.rpcid();
        auto iter = this->mRetActionMap.find(rpcId);
        if (iter == this->mRetActionMap.end())
        {
            return false;
        }
        CallHandler *callHandler = iter->second;
#ifdef __DEBUG__
        unsigned short methodId = response.methodid();
        auto component = this->GetComponent<ProtocolComponent>();
        auto config = component->GetProtocolConfig(methodId);
        double second = (TimeHelper::GetMilTimestamp() - callHandler->GetCreateTime()) / 1000.f;
        SayNoDebugLog("call " << config->ServiceName << "." << config->Method << " use time = " << second << "s");
#endif
        this->mNumberPool.Push(rpcId);
        callHandler->Invoke(response);
        this->mRetActionMap.erase(iter);
        return true;
    }
}
