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

    }

    bool RpcResponseComponent::Awake()
    {
        mTimeoutResponse.set_code((int)XCode::CallTimeout);
        GKAssertRetFalse_F(this->mTimerComponent = this->GetComponent<TimerComponent>());
        GKAssertRetFalse_F(this->mProtoComponent = this->GetComponent<RpcProtoComponent>());
        return true;
    }

	bool RpcResponseComponent::AddCallHandler(CallHandler * rpcAction, unsigned int & id)
    {
        GKAssertRetCode_F(rpcAction);
        int methodId = rpcAction->GetMethodId();
        auto config = this->mProtoComponent->GetProtocolConfig(methodId);
        if(config == nullptr)
        {
            return false;
        }
        id = this->mNumberPool.Pop();
        if(config->Timeout != 0)
        {
            this->mTimerComponent->AddTimer(config->Timeout, &RpcResponseComponent::OnTimeout, this, id);
        }
        rpcAction->SetTimerId(id);
        this->mRetActionMap.emplace(id, rpcAction);
        return id;
    }

    void RpcResponseComponent::OnTimeout(unsigned int id)
    {
        auto iter = this->mRetActionMap.find(id);
        if(iter != this->mRetActionMap.end())
        {
            CallHandler * callHandler = iter->second;

            this->mRetActionMap.erase(iter);
            if(callHandler != nullptr)
            {
                int methodId = callHandler->GetMethodId();
                callHandler->Invoke(this->mTimeoutResponse);
                auto config = this->mProtoComponent->GetProtocolConfig(methodId);
                GKDebugError("call ["<<config->Service << "." << config->Method << "] time out");
            }
        }
    }

	const CallHandler * RpcResponseComponent::GetRpcHandler(unsigned int rpcId) const
	{	
		auto iter = this->mRetActionMap.find(rpcId);
		return iter != this->mRetActionMap.end() ? iter->second : nullptr;
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
            this->mTimerComponent->RemoveTimer(callHandler->GetTimerId());
        }
        this->mRetActionMap.erase(iter);
        return true;
    }
}
