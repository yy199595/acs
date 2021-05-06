#include"AddressManager.h"
#include<CommonCore/Applocation.h>
#include<CommonNetWork/RemoteScheduler.h>
#include<CommonUtil/StringHelper.h>
#include<CommonManager/NetWorkManager.h>
#include<CommonProtocol/ServerCommon.pb.h>
#include<CommonCoroutine/CoroutineManager.h>
namespace SoEasy
{
	void AddressManager::OnInitComplete()
	{
		SessionManager::OnInitComplete();
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mTcpSessionListener->StartAcceptConnect();
		SayNoDebugInfo("start listen port : " << this->mTcpSessionListener->GetListenPort());
	}

	bool AddressManager::OnInit()
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
		REGISTER_FUNCTION_1(AddressManager::UpdateActionAddress, ActionUpdateInfo);

		mTcpSessionListener = make_shared<TcpSessionListener>(this, mListenPort);
		return mTcpSessionListener->InitListener();
	}

	void AddressManager::OnSecondUpdate()
	{
		SessionManager::OnSecondUpdate();
	}

	void AddressManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{		
		
	}

	void AddressManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		SayNoDebugWarning("connect new session : " << tcpSession->GetAddress());		
	}

	XCode AddressManager::UpdateActionAddress(shared_ptr<TcpClientSession> session, long long id, const ActionUpdateInfo & actions)
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
	XCode AddressManager::SyncActionInfos(int areaId)
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
					remoteScheduler.Call("NetWorkManager.UpdateAction", &returnData);
				}
			}		
		}
		return XCode::Successful;
	}
}
