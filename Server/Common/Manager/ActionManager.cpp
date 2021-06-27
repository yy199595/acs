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
		SharedPacket callbackData;
		this->mCallbackMessageQueue.SwapQueueData();
		while (this->mCallbackMessageQueue.PopItem(callbackData))
		{
			const long long callbackId = callbackData->rpcid();
			auto iter = this->mRetActionMap.find(callbackId);
			if (iter != this->mRetActionMap.end())
			{
				iter->second->Invoke(callbackData);
				this->mRetActionMap.erase(iter);
			}
		}
	}

	bool ActionManager::AddActionArgv(long long id, shared_ptr<NetWorkPacket> messageData)
	{
		auto iter = this->mRetActionMap.find(id);
		if (iter == this->mRetActionMap.end())
		{
			SayNoDebugError("not find callback " << id);
			return false;
		}
		this->mCallbackMessageQueue.AddItem(messageData);
		return true;
	}

	long long ActionManager::AddCallback(shared_ptr<LocalRetActionProxy> actionBox)
	{
		if (actionBox == nullptr)
		{
			return 0;
		}
		long long callbackId = NumberHelper::Create();
		this->mRetActionMap.emplace(callbackId, actionBox);
		if (this->mMessageTimeout != 0)// 添加超时
		{
			shared_ptr<ActionTimeoutTimer> timer = make_shared<ActionTimeoutTimer>(this->mMessageTimeout, callbackId, this);
			this->mTimerManager->AddTimer(timer);
		}
		this->mRetActionMap.emplace(callbackId, actionBox);
		return callbackId;
	}

}
