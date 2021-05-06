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
		REGISTER_FUNCTION_1(AddressManager::QueryActions, AreaActionInfo);
		REGISTER_FUNCTION_2(AddressManager::QueryAction, StringData, StringArray);
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

	XCode AddressManager::QueryAction(shared_ptr<TcpClientSession> session, long long id, const StringData & name, StringArray & returnData)
	{
		int areaId = (int)id;
		auto iter = this->mAreaActionMap.find(areaId);
		if (iter == this->mAreaActionMap.end())
		{
			return XCode::Failure;
		}
		std::vector<string> addressList;
		AreaActionTable * actionTable = iter->second;
		if (!actionTable->GetActionAddress(name.data(), addressList))
		{
			return XCode::Failure;
		}
		for (size_t index = 0; index < addressList.size(); index++)
		{
			const std::string & address = addressList[index];
			returnData.add_data_array()->assign(address);
		}
		return XCode::Successful;
	}

	XCode AddressManager::QueryActions(shared_ptr<TcpClientSession> session, long long id, AreaActionInfo & returnData)
	{
		int areaId = (int)id;
		auto iter = this->mAreaActionMap.find(areaId);
		if (iter == this->mAreaActionMap.end())
		{
			return XCode::Failure;
		}
		AreaActionTable * actionTable = iter->second;
		actionTable->ForeachActions([&returnData](const string & name, const std::set<std::string> & addressSet) ->bool
		{
			auto actionInfo = returnData.add_action_infos();
			actionInfo->set_action_name(name);
			for (auto iter = addressSet.begin(); iter != addressSet.end(); iter++)
			{
				const std::string & address = (*iter);
				actionInfo->add_action_address()->assign(address);
			}		
			return true;
		});

		return XCode::Successful;
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
		if (iter1 != this->mAddressAreaIdMap.end())
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
		return XCode::Successful;
	}
}
