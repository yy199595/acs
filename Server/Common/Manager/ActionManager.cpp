#include"ActionManager.h"
#include"ScriptManager.h"
#include"NetWorkManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Util/NumberHelper.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	ActionManager::ActionManager()
	{
		this->mScriptManager = nullptr;
		this->mNetWorkManager = nullptr;
	}

	bool ActionManager::OnInit()
	{
		this->mScriptManager = this->GetManager<ScriptManager>();
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mCoroutineScheduler = this->GetManager<CoroutineManager>();

		SayNoAssertRetFalse_F(this->mNetWorkManager);
		SayNoAssertRetFalse_F(this->mCoroutineScheduler);
		return true;
	}

	void ActionManager::OnDestory()
	{	
		for (auto iter = this->mRetActionMap.begin(); iter != this->mRetActionMap.end(); iter++)
		{
			NetWorkRetActionBox * pAction = iter->second;
			delete pAction;
		}

		for (auto iter = this->mRegisterActions.begin(); iter != this->mRegisterActions.end(); iter++)
		{
			NetWorkActionBox * pAction = iter->second;
			delete pAction;
		}
		for (auto iter = this->mRegisterLuaActions.begin(); iter != this->mRegisterLuaActions.end(); iter++)
		{
			NetLuaAction * pAction = iter->second;
			delete pAction;
		}
		this->mRetActionMap.clear();
		this->mRegisterActions.clear();
		this->mRegisterLuaActions.clear();
	}

	void ActionManager::GetAllFunction(std::vector<std::string>& funcs)
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

	bool ActionManager::BindFunction(NetLuaAction * actionBox)
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

	bool ActionManager::BindFunction(NetWorkActionBox * actionBox)
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

	long long ActionManager::AddCallback(NetWorkRetActionBox * actionBox)
	{
		if (actionBox != nullptr)
		{
			long long id = NumberHelper::Create();
			this->mRetActionMap.insert(std::make_pair(id, actionBox));
			return id;
		}
		return 0;
	}

	void ActionManager::OnSecondUpdate()
	{
		/*const long long nowTime = TimeHelper::GetSecTimeStamp();
		for (auto iter = this->mRetActionMap.begin(); iter != this->mRetActionMap.end();)
		{
			NetWorkRetActionBox * retActionBox = iter->second;
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

	void ActionManager::OnInitComplete()
	{
		for (auto iter = this->mRegisterActions.begin(); iter != this->mRegisterActions.end(); iter++)
		{
			const std::string & nFunctionName = iter->first;
			SayNoDebugInfo("Bind Function : " << nFunctionName);
		}
	}

	bool ActionManager::Call(shared_ptr<TcpClientSession> tcpSession, const long long id, const shared_ptr<NetWorkPacket> callInfo)
	{
		auto iter = this->mRetActionMap.find(id);
		if (iter == this->mRetActionMap.end())
		{
			SayNoDebugError("No callback method found " << id);
			return false;
		}
		NetWorkRetActionBox * refActionBox = iter->second;
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

	bool ActionManager::Call(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const shared_ptr<NetWorkPacket> callInfo)
	{
		NetLuaAction * pNetLuaAction = GetLuaFunction(name);
		if (pNetLuaAction != nullptr)
		{
			shared_ptr<NetWorkPacket> returnData = make_shared<NetWorkPacket>();
			pNetLuaAction->Invoke(tcpSession, callInfo, returnData);
			return true;
		}
		NetWorkActionBox * pAction = GetFunction(name);
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

	NetLuaAction * ActionManager::GetLuaFunction(const std::string & name)
	{
		auto iter = this->mRegisterLuaActions.find(name);
		return iter != this->mRegisterLuaActions.end() ? iter->second : nullptr;
	}

	NetWorkActionBox * ActionManager::GetFunction(const std::string & name)
	{
		auto iter = this->mRegisterActions.find(name);
		return iter != this->mRegisterActions.end() ? iter->second : nullptr;
	}
}
