#include"ActionQueryManager.h"
#include<Util/StringHelper.h>
#include<NetWork/RemoteScheduler.h>

#include<Coroutine/CoroutineManager.h>

#include<Manager/ActionManager.h>
#include<Manager/ListenerManager.h>
namespace SoEasy
{
	ActionQueryManager::ActionQueryManager()
	{
		this->mActionManager = nullptr;
		this->mListenerManager = nullptr;
		this->mCoroutineManager = nullptr;
	}

	bool ActionQueryManager::OnInit()
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
		this->mActionManager = this->GetManager<ActionManager>();
		this->mListenerManager = this->GetManager<ListenerManager>();
		SayNoAssertRetFalse_F(this->mActionManager);
		SayNoAssertRetFalse_F(this->mListenerManager);

		REGISTER_FUNCTION_1(ActionQueryManager::UpdateActions, PB::AreaActionInfo);
		this->mActionQuerySession = make_shared<TcpClientSession>(this, "QuerySession", mQueryIp, mQueryPort);
		return this->mActionQuerySession->StartConnect();
	}

	void ActionQueryManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void ActionQueryManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
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

	bool ActionQueryManager::GetActionAddress(const std::string & action, long long id, std::string & address)
	{
		auto iter = this->mActionAddressMap.find(action);
		if (iter == this->mActionAddressMap.end())
		{
			return false;
		}
		std::vector<std::string> & addressList = iter->second;
		if (addressList.empty())
		{
			return false;
		}
		address = addressList[id % addressList.size()];
		return true;
	}

	XCode ActionQueryManager::UpdateActions(shared_ptr<TcpClientSession> session, long long id, const PB::AreaActionInfo & actionInfos)
	{
		for (int index = 0; index < actionInfos.action_infos_size(); index++)
		{
			std::vector<std::string> addressVector;
			const AreaActionInfo_ActionInfo & actionInfo = actionInfos.action_infos(index);
			for (int i = 0; i < actionInfo.action_address_size(); i++)
			{
				const std::string & address = actionInfo.action_address(i);
				addressVector.emplace_back(address);
			}
			const std::string & name = actionInfo.action_name();
			this->mActionAddressMap.emplace(name, addressVector);
		}
		return XCode::Successful;
	}
}
