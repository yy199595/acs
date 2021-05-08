#include"ActionRegisterManager.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/RemoteScheduler.h>
#include<Protocol/ServerCommon.pb.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	void ActionRegisterManager::OnInitComplete()
	{
		SessionManager::OnInitComplete();
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mTcpSessionListener->StartAcceptConnect();
		SayNoDebugInfo("start listen port : " << this->mTcpSessionListener->GetListenPort());
	}

	bool ActionRegisterManager::OnInit()
	{
		std::string listenAddress;
		SayNoAssertRetFalse_F(SessionManager::OnInit());
		if (!this->GetConfig().GetValue("ActionQueryAddress", listenAddress))
		{
			SayNoDebugError("not find config field ActionQueryAddress");
			return false;
		}
		if (!StringHelper::ParseIpAddress(listenAddress, mListenIp, mListenPort))
		{
			SayNoDebugError("parse ActionQueryAddress fail");
			return false;
		}
		REGISTER_FUNCTION_1(ActionRegisterManager::RegisterActions, ActionUpdateInfo);

		mTcpSessionListener = make_shared<TcpSessionListener>(this, mListenPort);
		return mTcpSessionListener->InitListener();
	}

	void ActionRegisterManager::OnSecondUpdate()
	{
		SessionManager::OnSecondUpdate();
	}

	void ActionRegisterManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{		
		
	}

	void ActionRegisterManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		SayNoDebugWarning("connect new session : " << tcpSession->GetAddress());		
	}

	XCode ActionRegisterManager::RegisterActions(shared_ptr<TcpClientSession> session, long long id, const ActionUpdateInfo & actions)
	{
		int areaId = (int)id;
		const string & address = actions.address();
		auto iter = this->mAreaActionMap.find(areaId);
		if (iter == this->mAreaActionMap.end())
		{
			AreaActionTable * areaTable = new AreaActionTable(areaId);
			this->mAreaActionMap.emplace(areaId, areaTable);
		}
		const string & sessionAddress = session->GetAddress();
		auto iter1 = this->mAddressAreaIdMap.find(sessionAddress);
		if (iter1 == this->mAddressAreaIdMap.end())
		{
			this->mAddressAreaIdMap.emplace(sessionAddress, areaId);
		}
		
		AreaActionTable * areaTable = this->mAreaActionMap[areaId];
		
		for (int index = 0; index < actions.action_names_size(); index++)
		{
			const std::string & name = actions.action_names(index);
			areaTable->AddActionAddress(name, address);
			for (auto iter = this->mAreaActionMap.begin(); iter != this->mAreaActionMap.end() && areaId == 0; iter++)
			{
				AreaActionTable * commonAreaTable = iter->second;
				commonAreaTable->AddActionAddress(name, address);
			}
			SayNoDebugWarning(name << "  " << address);
		}	
		return this->SyncActionInfos(areaId);
	}
	XCode ActionRegisterManager::SyncActionInfos(int areaId)
	{
		auto iter1 = this->mAreaActionMap.find(areaId);
		if (iter1 == this->mAreaActionMap.end())
		{
			return XCode::Failure;
		}
		AreaActionInfo returnData;
		AreaActionTable * areaTable = this->mAreaActionMap[areaId];
		areaTable->ForeachActions([&returnData](const std::string & name, const std::set<string> & addressList)->bool
		{
			AreaActionInfo_ActionInfo * actionInfo = returnData.add_action_infos();
			if (actionInfo != nullptr)
			{
				actionInfo->set_action_name(name);
				for (auto iter = addressList.begin(); iter != addressList.end(); iter++)
				{
					actionInfo->add_action_address()->assign(*iter);
				}
			}	
			return true;
		});
		for (auto iter = this->mAddressAreaIdMap.begin(); iter != this->mAddressAreaIdMap.end(); iter++)
		{
			if (iter->second == areaId)
			{
				const string & address = iter->first;		
				shared_ptr<TcpClientSession> tcpSession = this->mNetWorkManager->GetSessionByAdress(address);
				if (tcpSession != nullptr)
				{
					RemoteScheduler remoteScheduler(tcpSession);
					remoteScheduler.Call("ActionQueryManager.UpdateActions", &returnData);
				}
			}		
		}
		return XCode::Successful;
	}
}
