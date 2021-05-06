#include"ActionManager.h"
#include"ScriptManager.h"
#include"NetWorkManager.h"
#include<CommonUtil/StringHelper.h>
#include<CommonCore/Applocation.h>
#include<CommonUtil/NumberHelper.h>
#include<CommonNetWork/RemoteScheduler.h>
#include<CommonNetWork/NetWorkRetAction.h>
#include<CommonManager/ListenerManager.h>
namespace SoEasy
{
	ActionManager::ActionManager()
	{
		this->mScriptManager = nullptr;
		this->mNetWorkManager = nullptr;
	}

	bool ActionManager::OnInit()
	{
		std::string connectAddress;
		if (!SessionManager::OnInit())
		{
			return false;
		}
		if (!this->GetConfig().GetValue("AreaId", this->mAreaId))
		{
			SayNoDebugError("not find field AreaId");
			return false;
		}
		if (!this->GetConfig().GetValue("ActionQueryAddress", connectAddress))
		{
			SayNoDebugError("not find field ActionQueryAddress");
			return false;
		}
		if (!StringHelper::ParseIpAddress(connectAddress, mQueryIp, mQueryPort))
		{
			SayNoDebugFatal("parse ActionQueryAddress fail");
			return false;
		}
		this->mScriptManager = this->GetManager<ScriptManager>();
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mListenerManager = this->GetManager<ListenerManager>();

		SayNoAssertRetFalse_F(this->mNetWorkManager);
		SayNoAssertRetFalse_F(this->mListenerManager);
		
		this->mActionQuerySession = make_shared<TcpClientSession>(this, "QuerySession", mQueryIp, mQueryPort);
		return this->mActionQuerySession->StartConnect();
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

	void ActionManager::OnFrameUpdateAfter()
	{
		while (!this->mWaitDestoryActions.empty())
		{
			NetWorkRetActionBox * action = this->mWaitDestoryActions.front();
			this->mWaitDestoryActions.pop();
			delete action;
		}
	}

	bool ActionManager::Call(shared_ptr<TcpClientSession> tcpSession, const long long id, const NetWorkPacket & callInfo)
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
		this->mWaitDestoryActions.push(refActionBox);
		this->mRetActionMap.erase(iter);
		return true;
	}

	bool ActionManager::Call(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const NetWorkPacket & callInfo)
	{
		NetWorkPacket returnData;
		const long long nowTime = TimeHelper::GetMilTimestamp();
		XCode code = this->CallBindFunction(tcpSession, callInfo, returnData);
		if (callInfo.callback_id() != 0)
		{
			const std::string & address = tcpSession->GetAddress();
			returnData.set_error_code(code);
			returnData.set_callback_id(callInfo.callback_id());
			returnData.set_operator_id(callInfo.operator_id());
			return mNetWorkManager->SendMessageByAdress(address, make_shared<NetWorkPacket>(returnData));
		}
		return true;
	}

	NetLuaAction * ActionManager::GetOrFindLuaFunction(const std::string & name)
	{
		if (this->mScriptManager == nullptr)
		{
			return nullptr;
		}

		auto iter = this->mRegisterLuaActions.find(name);
		if (iter != this->mRegisterLuaActions.end())
		{
			NetLuaAction * pLuaFunction = iter->second;
			return pLuaFunction;
		}

		const size_t pos = name.find_first_of(".");
		if (pos == std::string::npos)
		{
			return nullptr;
		}
		const std::string className = name.substr(0, pos);
		const std::string funcName = name.substr(pos + 1, name.length());
		lua_State * luaEnv = this->mScriptManager->GetLuaEnv();
		NetLuaAction * pLuaFunction = NetLuaAction::Create(luaEnv, className, funcName);
		if (pLuaFunction == nullptr)
		{
			return nullptr;
		}
		this->mRegisterLuaActions.insert(std::make_pair(name, pLuaFunction));
		return pLuaFunction;
	}

	XCode ActionManager::CallBindFunction(shared_ptr<TcpClientSession> tcpSession, const NetWorkPacket & callFunctionData, NetWorkPacket & returnData)
	{
		returnData.Clear();
		const std::string & name = callFunctionData.func_name();
		auto iter = this->mRegisterActions.find(name);
		if (iter != this->mRegisterActions.end())
		{
			NetWorkActionBox * bindActionBox = iter->second;
			return bindActionBox->Invoke(tcpSession, callFunctionData, returnData);
		}

		NetLuaAction * pLuaFunction = this->GetOrFindLuaFunction(name);
		if (pLuaFunction == nullptr)
		{
			SayNoDebugError("Call Function " << name << " Not Exit");
			return XCode::CallFunctionNotExist;
		}
		return pLuaFunction->Invoke(tcpSession, callFunctionData, returnData);
	}

	void ActionManager::OnLoadLuaComplete(lua_State * luaEnv)
	{
		auto iter = this->mRegisterActions.begin();
		for (; iter != this->mRegisterActions.end(); iter++)
		{
			NetWorkActionBox * pActionBox = iter->second;

			const std::string & name = iter->first;
			const size_t pos = name.find_first_of(".");
			const std::string className = name.substr(0, pos);
			const std::string funcName = name.substr(pos + 1, name.length());
			NetLuaAction * pLuaFunction = NetLuaAction::Create(luaEnv, className, funcName);
			if (pLuaFunction != nullptr)
			{
				pActionBox->BindLuaFunction(pLuaFunction);
				SayNoDebugInfo("bind lua function " << name);
			}
		}
		auto iter1 = this->mRegisterLuaActions.begin();
		for (; iter1 != this->mRegisterLuaActions.end(); iter1++)
		{
			NetLuaAction * pLuaFunction = iter1->second;
			if (pLuaFunction != nullptr)
			{
				delete pLuaFunction;
			}
		}
		this->mRegisterLuaActions.clear();
	}

	void ActionManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void ActionManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		PB::ActionUpdateInfo actionInfo;
		RemoteScheduler remoteScheduler(tcpSession, this->mAreaId);
		const std::string & address = this->mListenerManager->GetAddress();
		SayNoDebugWarning("connect address manager success " << tcpSession->GetAddress());
		actionInfo.set_address(address);
		for (auto iter = this->mRegisterActions.begin(); iter != this->mRegisterActions.end(); iter++)
		{
			const std::string & actionName = iter->first;
			actionInfo.add_action_names()->assign(actionName);
		}
		remoteScheduler.Call("AddressManager.UpdateActionAddress", &actionInfo, [](shared_ptr<TcpClientSession>, XCode code)
		{

		});
	}
}
