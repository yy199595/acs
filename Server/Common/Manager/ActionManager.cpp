#include"ActionManager.h"
#include"ScriptManager.h"
#include"NetWorkManager.h"
#include"TimerManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<NetWork/NetLuaAction.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Timer/ActionTimeoutTimer.h>
namespace SoEasy
{
	ActionManager::ActionManager()
	{
		this->mMessageTimeout = 0;
		this->mScriptManager = nullptr;
		this->mNetWorkManager = nullptr;
	}

	bool ActionManager::OnInit()
	{
		this->mScriptManager = this->GetManager<ScriptManager>();
		this->GetConfig().GetValue("MsgTimeout", this->mMessageTimeout);
		SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mCoroutineScheduler = this->GetManager<CoroutineManager>());
		return true;
	}

	void ActionManager::OnDestory()
	{	
		this->mRetActionMap.clear();
		this->mRegisterLuaActions.clear();
	}

	bool ActionManager::BindFunction(shared_ptr<NetLuaAction> actionBox)
	{
		if (actionBox == nullptr)
		{
			return false;
		}
		const std::string & name = actionBox->GetActionName();
		if (this->mRegisterLuaActions.find(name) != this->mRegisterLuaActions.end())
		{
			SayNoDebugError("Repeated registration action : " << name);
			return false;
		}
		SayNoDebugInfo("add lua action " << name << " successful");
		this->mRegisterLuaActions.insert(std::make_pair(name, actionBox));
		return true;
	}

	bool ActionManager::DelCallback(long long callbackId)
	{
		auto iter = this->mRetActionMap.find(callbackId);
		if (iter != this->mRetActionMap.end())
		{
			this->mRetActionMap.erase(iter);
			return true;
		}
		return false;
	}

	shared_ptr<LocalRetActionProxy> ActionManager::GetCallback(long long callbackId, bool remove)
	{
		auto iter = this->mRetActionMap.find(callbackId);
		if (iter != this->mRetActionMap.end())
		{
			shared_ptr<LocalRetActionProxy> actionBox = iter->second;
			if (remove)
			{
				this->mRetActionMap.erase(iter);
			}		
			return actionBox;
		}
		return nullptr;
	}


	bool ActionManager::AddCallback(shared_ptr<LocalRetActionProxy> actionBox, long long & callbackId)
	{
		if (actionBox == nullptr)
		{
			return false;
		}
		callbackId = NumberHelper::Create();
		this->mRetActionMap.emplace(callbackId, actionBox);
		if (this->mMessageTimeout != 0)
		{
			shared_ptr<ActionTimeoutTimer> timer = make_shared<ActionTimeoutTimer>(this->mMessageTimeout, callbackId, this);
			this->mTimerManager->AddTimer(timer);
		}
		// Ìí¼Ó³¬Ê±
		return true;
	}

	void ActionManager::OnInitComplete()
	{
		
	}

	shared_ptr<NetLuaAction> ActionManager::GetLuaAction(const std::string & name)
	{
		if (this->mScriptManager != nullptr)
		{
			auto iter = this->mRegisterLuaActions.find(name);
			return iter != this->mRegisterLuaActions.end() ? iter->second : nullptr;
		}
		return nullptr;
	}
}
