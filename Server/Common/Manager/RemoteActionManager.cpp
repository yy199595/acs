#include"RemoteActionManager.h"
#include<Util/StringHelper.h>
#include<NetWork/RemoteScheduler.h>

#include<Coroutine/CoroutineManager.h>

#include<Manager/LocalActionManager.h>
#include<Manager/ListenerManager.h>
namespace SoEasy
{
	RemoteActionManager::RemoteActionManager()
	{
		this->mActionManager = nullptr;
		this->mListenerManager = nullptr;
		this->mCoroutineManager = nullptr;
	}

	bool RemoteActionManager::OnInit()
	{
		if (!SessionManager::OnInit())
		{
			return false;
		}
		if (!this->GetConfig().GetValue("AreaId", this->mAreaId))
		{
			SayNoDebugError("not find field AreaId");
			return false;
		}
		std::string connectAddress;
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
		this->mActionManager = this->GetManager<LocalActionManager>();
		this->mListenerManager = this->GetManager<ListenerManager>();
		SayNoAssertRetFalse_F(this->mActionManager);
		SayNoAssertRetFalse_F(this->mListenerManager);

		REGISTER_FUNCTION_1(RemoteActionManager::UpdateActions, PB::AreaActionInfo);
		this->mActionQuerySession = make_shared<TcpClientSession>(this, "QuerySession", mQueryIp, mQueryPort);
		return this->mActionQuerySession->StartConnect();
	}

	void RemoteActionManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void RemoteActionManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		PB::ActionUpdateInfo actionInfo;
		RemoteScheduler remoteScheduler(tcpSession, this->mAreaId);
		const std::string & address = this->mListenerManager->GetAddress();
		SayNoDebugWarning("connect address manager success " << tcpSession->GetAddress());
		actionInfo.set_address(address);

		std::vector<std::string> actions;
		this->mActionManager->GetAllFunction(actions);

		for (size_t index = 0; index < actions.size(); index++)
		{
			const std::string & actionName = actions[index];
			actionInfo.add_action_names()->assign(actionName);
		}

		remoteScheduler.Call("ActionRegisterManager.RegisterActions", &actionInfo, [](shared_ptr<TcpClientSession>, XCode code)
		{
			SayNoDebugInfo("register action successful");
		});
	}

	RemoteActionProxy * RemoteActionManager::GetActionProxy(const std::string & action, long long id)
	{
		auto iter = this->mActionAddressMap.find(action);
		if (iter == this->mActionAddressMap.end())
		{
			return nullptr;
		}
		std::vector<std::string> & addressList = iter->second;
		if (addressList.empty())
		{
			return nullptr;
		}
		std::string & address = addressList[id % addressList.size()];
		auto iter1 = this->mActionProxyMap.find(address);
		return iter1 != this->mActionProxyMap.end() ? iter1->second : nullptr;
	}

	XCode RemoteActionManager::UpdateActions(shared_ptr<TcpClientSession> session, long long id, shared_ptr<PB::AreaActionInfo> actionInfos)
	{
		std::set<std::string> allActionAddress;
		for (int index = 0; index < actionInfos->action_infos_size(); index++)
		{
			std::vector<std::string> actionAddressVector;
			const AreaActionInfo_ActionInfo & actionInfo = actionInfos->action_infos(index);
			const std::string & name = actionInfo.action_name();
			for (int i = 0; i < actionInfo.action_address_size(); i++)
			{
				const std::string & address = actionInfo.action_address(i);

				allActionAddress.insert(address);
				actionAddressVector.push_back(address);
			}	
			this->mActionAddressMap.emplace(name, actionAddressVector);
		}

		for (const std::string & address : allActionAddress)
		{
			RemoteActionProxy * actionProxy = new RemoteActionProxy(this->mNetWorkManager, address);
			this->mActionProxyMap.insert(std::make_pair(address, actionProxy));
		}

		return XCode::Successful;
	}
}
