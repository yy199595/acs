#include"LocalActionManager.h"
#include"ScriptManager.h"
#include"NetWorkManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<NetWork/NetLuaAction.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	LocalActionManager::LocalActionManager()
	{
		this->mScriptManager = nullptr;
		this->mNetWorkManager = nullptr;
	}

	bool LocalActionManager::OnInit()
	{
		this->mScriptManager = this->GetManager<ScriptManager>();
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mCoroutineScheduler = this->GetManager<CoroutineManager>();

		SayNoAssertRetFalse_F(this->mNetWorkManager);
		SayNoAssertRetFalse_F(this->mCoroutineScheduler);
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
		return true;
	}

	void LocalActionManager::OnSecondUpdate()
	{
		/*const long long nowTime = TimeHelper::GetSecTimeStamp();
		for (auto iter = this->mRetActionMap.begin(); iter != this->mRetActionMap.end();)
		{
			LocalRetActionProxy * retActionBox = iter->second;
			if (retActionBox != nullptr && nowTime >= retActionBox->GetInitiativeTime())
			{
				this->mRetActionMap.erase(iter++);
				const std::string message = "time out auto call";
				retActionBox->Invoke(nullptr, XCode::TimeoutAutoCall, message);
				SayNoDebugLog("call " << retActionBox->GetFunctionName() << " time out");
				delete retActionBox;
				continue;
			}
			iter++;
		}*/
	}

	void LocalActionManager::OnInitComplete()
	{
		for (auto iter = this->mRegisterActions.begin(); iter != this->mRegisterActions.end(); iter++)
		{
			const std::string & nFunctionName = iter->first;
			SayNoDebugInfo("Bind Function : " << nFunctionName);
		}
	}

	/*bool LocalActionManager::Call(shared_ptr<TcpClientSession> tcpSession, const long long id, const shared_ptr<NetWorkPacket> callInfo)
	{
		auto iter = this->mRetActionMap.find(id);
		if (iter == this->mRetActionMap.end())
		{
			SayNoDebugError("No callback method found " << id);
			return false;
		}
		LocalRetActionProxy * refActionBox = iter->second;
		if (refActionBox == nullptr)
		{
			this->mRetActionMap.erase(iter);
			return false;
		}

		refActionBox->Invoke(tcpSession, callInfo);
		this->mRetActionMap.erase(iter);
		delete refActionBox;
		return true;
	}

	bool LocalActionManager::Call(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const shared_ptr<NetWorkPacket> callInfo)
	{
		NetLuaAction * pNetLuaAction = GetLuaFunction(name);
		if (pNetLuaAction != nullptr)
		{
			shared_ptr<NetWorkPacket> returnData = make_shared<NetWorkPacket>();
			pNetLuaAction->Invoke(tcpSession, callInfo, returnData);
			return true;
		}
		LocalActionProxy * pAction = GetFunction(name);
		if (pAction == nullptr)
		{
			return false;
		}

		this->mCoroutineScheduler->Start([this, callInfo, pAction, tcpSession]()
		{
			shared_ptr<NetWorkPacket> returnData = make_shared<NetWorkPacket>();
			const long long nowTime = TimeHelper::GetMilTimestamp();
			XCode code = pAction->Invoke(tcpSession, callInfo, returnData);
			if (callInfo->callback_id() != 0)
			{
				const std::string & address = tcpSession->GetAddress();
				returnData->set_error_code(code);
				returnData->set_callback_id(callInfo->callback_id());
				returnData->set_operator_id(callInfo->operator_id());
				mNetWorkManager->SendMessageByAdress(address, returnData);
			}
		});
		return true;
	}
*/
	shared_ptr<NetLuaAction> LocalActionManager::GetLuaAction(const std::string & name)
	{
		auto iter = this->mRegisterLuaActions.find(name);
		return iter != this->mRegisterLuaActions.end() ? iter->second : nullptr;
	}

	shared_ptr<LocalActionProxy> LocalActionManager::GetAction(const std::string & name)
	{
		auto iter = this->mRegisterActions.find(name);
		return iter != this->mRegisterActions.end() ? iter->second : nullptr;
	}
}
