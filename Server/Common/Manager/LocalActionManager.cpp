#include"LocalActionManager.h"
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
	LocalActionManager::LocalActionManager()
	{
		this->mMessageTimeout = 0;
		this->mScriptManager = nullptr;
		this->mNetWorkManager = nullptr;
	}

	bool LocalActionManager::OnInit()
	{
		this->mScriptManager = this->GetManager<ScriptManager>();
		this->GetConfig().GetValue("MsgTimeout", this->mMessageTimeout);
		SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mCoroutineScheduler = this->GetManager<CoroutineManager>());
		return true;
	}

	void LocalActionManager::OnDestory()
	{	
		this->mRetActionMap.clear();
		this->mRegisterActions.clear();
		this->mRegisterLuaActions.clear();
	}

	void LocalActionManager::GetAllFunction(std::vector<std::string>& funcs)
	{
		funcs.clear();
		for (auto iter = this->mRegisterActions.begin(); iter != this->mRegisterActions.end(); iter++)
		{
			funcs.emplace_back(iter->first);
		}

		for (auto iter = this->mRegisterLuaActions.begin(); iter != this->mRegisterLuaActions.end(); iter++)
		{
			funcs.emplace_back(iter->first);
		}
	}

	bool LocalActionManager::BindFunction(shared_ptr<NetLuaAction> actionBox)
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

	bool LocalActionManager::BindFunction(shared_ptr<LocalActionProxy> actionBox)
	{
		if (actionBox == nullptr)
		{
			return false;
		}
		const std::string & name = actionBox->GetName();
		if (this->mRegisterActions.find(name) != this->mRegisterActions.end())
		{
			SayNoDebugError("Repeated registration action : " << name);
			return false;
		}
		SayNoDebugInfo("add action " << name << " successful");
		this->mRegisterActions.insert(std::make_pair(name, actionBox));
		return true;
	}

	bool LocalActionManager::DelCallback(long long callbackId)
	{
		auto iter = this->mRetActionMap.find(callbackId);
		if (iter != this->mRetActionMap.end())
		{
			this->mRetActionMap.erase(iter);
			return true;
		}
		return false;
	}

	shared_ptr<LocalRetActionProxy> LocalActionManager::GetCallback(long long callbackId, bool remove)
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

	bool LocalActionManager::AddCallback(shared_ptr<LocalRetActionProxy> actionBox, long long & callbackId)
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

	void LocalActionManager::OnSecondUpdate()
	{
		
	}

	void LocalActionManager::OnInitComplete()
	{
		for (auto iter = this->mRegisterActions.begin(); iter != this->mRegisterActions.end(); iter++)
		{
			const std::string & nFunctionName = iter->first;
			SayNoDebugInfo("Bind Function : " << nFunctionName);
		}
	}

	shared_ptr<NetLuaAction> LocalActionManager::GetLuaAction(const std::string & name)
	{
		if (this->mScriptManager != nullptr)
		{
			auto iter = this->mRegisterLuaActions.find(name);
			return iter != this->mRegisterLuaActions.end() ? iter->second : nullptr;
		}
		return nullptr;
	}

	shared_ptr<LocalActionProxy> LocalActionManager::GetAction(const std::string & name)
	{
		auto iter = this->mRegisterActions.find(name);
		return iter != this->mRegisterActions.end() ? iter->second : nullptr;
	}
}
