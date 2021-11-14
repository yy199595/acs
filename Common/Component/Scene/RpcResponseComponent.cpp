#include"RpcResponseComponent.h"
#include<Util/StringHelper.h>
#include<Core/App.h>
#include<Util/NumberHelper.h>
#include<Method/CallHandler.h>
#include<Timer/ActionTimeoutTimer.h>

#ifdef __DEBUG__
#include<Method/CallHandler.h>
#include <Scene/RpcProtoComponent.h>
#endif
namespace GameKeeper
{
    RpcResponseComponent::RpcResponseComponent()
        : mNumberPool(100)
    {
        this->mMessageTimeout = 0;
    }

    bool RpcResponseComponent::Awake()
    {
		ServerConfig & config = App::Get().GetConfig();
		this->mTimerComponent = App::Get().GetTimerComponent();
		GKAssertRetFalse_F(config.GetValue("NetWork", "MsgTimeout", this->mMessageTimeout));
        return true;
    }

	bool RpcResponseComponent::AddCallHandler(CallHandler * rpcAction, unsigned int & id)
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

	const CallHandler * RpcResponseComponent::GetRpcHandler(unsigned int rpcId) const
	{	
		auto iter = this->mRetActionMap.find(rpcId);
		return iter == this->mRetActionMap.end() ? nullptr : iter->second;
	}

	bool RpcResponseComponent::OnResponse(const com::Rpc_Response & response)
    {
        unsigned int rpcId = response.rpcid();
        auto iter = this->mRetActionMap.find(rpcId);
        if (iter == this->mRetActionMap.end())
        {
            return false;
        }
		this->mNumberPool.Push(rpcId);
        CallHandler *callHandler = iter->second;
		if (callHandler != nullptr)
		{
			callHandler->Invoke(response);
		}      
        this->mRetActionMap.erase(iter);
        return true;
    }
}
