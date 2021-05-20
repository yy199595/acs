#include"ServiceQuery.h"
#include<Util/StringHelper.h>
#include<NetWork/ActionScheduler.h>
#include<NetWork/RemoteScheduler.h>
#include<Coroutine/CoroutineManager.h>
#include<Manager/ActionManager.h>
#include<Manager/ListenerManager.h>
namespace SoEasy
{
	ServiceQuery::ServiceQuery()
	{
		this->mActionManager = nullptr;
		this->mListenerManager = nullptr;
		this->mCoroutineManager = nullptr;
	}

	bool ServiceQuery::OnInit()
	{	
		SayNoAssertRetFalse_F(SessionManager::OnInit());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ActionQueryAddress", mQueryAddress));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(mQueryAddress, mQueryIp, mQueryPort));
		
		SayNoAssertRetFalse_F(this->mListenerManager = this->GetManager<ListenerManager>());
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());
		this->mActionQuerySession = make_shared<TcpClientSession>(this, "QuerySession", mQueryIp, mQueryPort);
		return true;
	}

	void ServiceQuery::OnInitComplete()
	{
		SayNoAssertRet_F(this->mActionQuerySession->StartConnect());
		SayNoDebugInfo("connect query address " << mQueryIp << ":" << mQueryPort);
	}

	void ServiceQuery::OnSessionErrorAfter(SharedTcpSession tcpSession)
	{

	}

	void ServiceQuery::OnSessionConnectAfter(SharedTcpSession tcpSession)
	{
		if (tcpSession == this->mActionQuerySession)
		{
			this->StartRegisterAction();
			SayNoDebugInfo("start register location action to " << tcpSession->GetAddress());
		}
		else
		{
			const std::string & address = tcpSession->GetAddress();
			std::vector<shared_ptr<RemoteActionProxy>> actionProxys;
			this->GetActionProxyByAddress(address, actionProxys);
			for (size_t index = 0; index < actionProxys.size(); index++)
			{
				shared_ptr<RemoteActionProxy> actionProxy = actionProxys[index];
				actionProxy->BindSession(tcpSession);
			}
		}
	}

	bool ServiceQuery::GetActionProxy(const std::string & action, shared_ptr<RemoteActionProxy> & actionProxy)
	{
		auto iter = this->mActionProxyMap.find(action);
		if (iter == this->mActionProxyMap.end() || iter->second.empty())
		{
			this->StartPullActionList();
			return false;
		}
		actionProxy = iter->second[0];
		return actionProxy->IsAction() ? true : this->StartConnectToAction(actionProxy);
	}

	bool ServiceQuery::GetActionProxy(const std::string & action, std::vector<shared_ptr<RemoteActionProxy>> & actionProxys)
	{
		auto iter = this->mActionProxyMap.find(action);
		if (iter == this->mActionProxyMap.end() || iter->second.empty())
		{
			this->StartPullActionList();
			return false;
		}
		for (size_t index = 0; index < iter->second.size(); index++)
		{
			shared_ptr<RemoteActionProxy> actionPrixy = iter->second[index];
			if (!actionPrixy->IsAction() && !this->StartConnectToAction(actionPrixy))
			{
				return false;
			}
		}
		actionProxys = iter->second;
		return true;
	}

	bool ServiceQuery::GetActionProxy(const std::string & action, long long operId, shared_ptr<RemoteActionProxy> & actionProxy)
	{
		auto iter = this->mActionProxyMap.find(action);
		if (iter == this->mActionProxyMap.end() && iter->second.empty())
		{
			this->StartPullActionList();
			return false;
		}
		//TODO
		return true;
	}

	void ServiceQuery::GetActionProxyByAddress(const std::string & address, std::vector<shared_ptr<RemoteActionProxy>>& actionProxys)
	{
		for (auto iter = this->mActionProxyMap.begin(); iter != this->mActionProxyMap.end(); iter++)
		{
			for (size_t index = 0; index < iter->second.size(); index++)
			{
				shared_ptr<RemoteActionProxy> actionProxy = iter->second[index];
				if (actionProxy->GetActionAddress() == address)
				{
					actionProxys.push_back(actionProxy);
				}
			}
		}
	}

	void ServiceQuery::AddNewActionProxy(int argaId, const std::string & name, const std::string & address)
	{
		auto iter = this->mActionProxyMap.find(name);
		if (iter == this->mActionProxyMap.end())
		{
			std::vector<shared_ptr<RemoteActionProxy>> actions;
			this->mActionProxyMap.emplace(name, actions);
		}
		std::vector<shared_ptr<RemoteActionProxy>> & actionList = this->mActionProxyMap[name];
		for (size_t index = 0; index < actionList.size(); index++)
		{
			if (actionList[index]->GetActionAddress() == address)
			{
				return;
			}
		}
		shared_ptr<RemoteActionProxy> actionProxy = make_shared<RemoteActionProxy>(name, address, argaId);
		actionList.push_back(actionProxy);
	}
	void ServiceQuery::StartRegisterAction()
	{
		PB::ActionUpdateInfo actionInfo;
		RemoteScheduler quertShceudler(this->mActionQuerySession);
		const std::string & address = this->mListenerManager->GetAddress();	
		
		actionInfo.set_address(address);
		actionInfo.set_areaid(mAreaId);
		std::vector<ServiceBase *> services;
		this->GetApp()->GetServices(services);
		
		for (size_t index = 0; index < services.size(); index++)
		{
			ServiceBase * service = services[index];
			actionInfo.add_action_names()->assign(service->GetTypeName());
		}
		// ×¢²á±¾µØactionµ½Ô¶¶Ë
		quertShceudler.Call("ServiceRegistry", "RegisterActions", &actionInfo);
	}
	void ServiceQuery::StartPullActionList()
	{
		Int32Data requestData;
		requestData.set_data(this->mAreaId);
		RemoteScheduler quertShceudler(this->mActionQuerySession);
		quertShceudler.Call<ActionInfoList>("ServiceRegistry", "QueryActions", &requestData,
			[this](XCode code, const PB::ActionInfoList & returnData)
		{
			for (int index = 0; index < returnData.actionlist_size(); index++)
			{
				const ActionInfo & actionInfo = returnData.actionlist(index);
				const int actionAreaId = actionInfo.adreid();
				const std::string & actionName = actionInfo.actionname();
				const std::string & actionAddress = actionInfo.address();
				this->AddNewActionProxy(actionAreaId, actionName, actionAddress);
				SayNoDebugWarning(actionAreaId << "\t" << actionAddress << "\t" << actionName);
			}
		});
	}
	bool ServiceQuery::StartConnectToAction(shared_ptr<RemoteActionProxy> actionProxy)
	{
		shared_ptr<TcpClientSession> tcpClientSession;
		const std::string & address = actionProxy->GetActionAddress();
		auto iter = this->mActionSessionMap.find(address);
		if (iter != this->mActionSessionMap.end())
		{
			tcpClientSession = iter->second;
			if (tcpClientSession->IsActive())
			{
				return true;
			}
		}
		else
		{
			std::string ip;
			unsigned short port;
			SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(address, ip, port));
			tcpClientSession = make_shared<TcpClientSession>(this, "ActionSession", ip, port);
			this->mActionSessionMap.emplace(address, tcpClientSession);
		}
		return tcpClientSession->StartConnect();
	}
}
