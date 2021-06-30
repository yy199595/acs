#include"ActionManager.h"
#include"ScriptManager.h"
#include"NetWorkManager.h"
#include"TimerManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Timer/ActionTimeoutTimer.h>
namespace SoEasy
{
	ActionManager::ActionManager()
	{
		this->mMessageTimeout = 0;
	}

	bool ActionManager::OnInit()
	{
		this->GetConfig().GetValue("MsgTimeout", this->mMessageTimeout);
		SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());
		return true;
	}

	void ActionManager::OnSystemUpdate()
	{
		SharedPacket responseData;
		this->mRemoteMessageQueue.SwapQueueData();
		while (this->mRemoteMessageQueue.PopItem(responseData))
		{
			long long rpcId = responseData->rpcid();
			auto iter = this->mRetActionMap.find(rpcId);
			if (iter != this->mRetActionMap.end())
			{
				iter->second->Invoke(responseData);
				this->mRetActionMap.erase(iter);
			}
		}
		while (!this->mLocalMessageQueue.empty())
		{
			responseData = this->mLocalMessageQueue.front();		
			long long rpcId = responseData->rpcid();
			auto iter = this->mRetActionMap.find(rpcId);
			if (iter != this->mRetActionMap.end())
			{
				iter->second->Invoke(responseData);
				this->mRetActionMap.erase(iter);
			}
			this->mLocalMessageQueue.pop();
		}
	}

	bool ActionManager::PushLocalResponseData(shared_ptr<NetWorkPacket> messageData)
	{
		long long id = messageData->rpcid();
		auto iter = this->mRetActionMap.find(id);
		if (iter == this->mRetActionMap.end())
		{
			SayNoDebugError("not find rpc cb " << id);
			return false;
		}
		this->mLocalMessageQueue.push(messageData);
		return true;
	}
	bool ActionManager::PushRemoteResponseData(shared_ptr<NetWorkPacket> messageData)
	{
		if (messageData->rpcid() == 0)
		{
			return false;
		}
		this->mRemoteMessageQueue.AddItem(messageData);
		return true;
	}

	long long ActionManager::AddCallback(shared_ptr<LocalRetActionProxy> rpcAction)
	{
		if (rpcAction == nullptr)
		{
			return 0;
		}
		long long callbackId = NumberHelper::Create();
		this->mRetActionMap.emplace(callbackId, rpcAction);
		if (this->mMessageTimeout != 0)// 添加超时
		{
			shared_ptr<ActionTimeoutTimer> timer = make_shared<ActionTimeoutTimer>(this->mMessageTimeout, callbackId, this);
			this->mTimerManager->AddTimer(timer);
		}
		this->mRetActionMap.emplace(callbackId, rpcAction);
		return callbackId;
	}
}
