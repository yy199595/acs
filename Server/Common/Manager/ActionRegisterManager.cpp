#include"ActionRegisterManager.h"
#include<Core/Applocation.h>
#include<Core/TcpSessionListener.h>
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/RemoteScheduler.h>
#include<Protocol/ServerCommon.pb.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	bool ActionRegisterManager::OnInit()
	{
		std::string listenAddress;
		SayNoAssertRetFalse_F(SessionManager::OnInit());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ActionQueryAddress", listenAddress));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(listenAddress, mListenIp, mListenPort));
	
		REGISTER_FUNCTION_1(ActionRegisterManager::RegisterActions, ActionUpdateInfo);
		REGISTER_FUNCTION_2(ActionRegisterManager::QueryActions, Int32Data, ActionInfoList);

		mTcpSessionListener = make_shared<TcpSessionListener>(this, mListenPort);
		SayNoAssertRetFalse_F(mTcpSessionListener->InitListener());
		return true;
	}

	void ActionRegisterManager::OnInitComplete()
	{
		SessionManager::OnInitComplete();
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mTcpSessionListener->StartAcceptConnect();
		SayNoDebugInfo("start listen port : " << this->mTcpSessionListener->GetListenPort());
	}

	void ActionRegisterManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{		
		
	}

	void ActionRegisterManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		SayNoDebugWarning("connect new session : " << tcpSession->GetAddress());		
	}

	XCode ActionRegisterManager::RegisterActions(long long operId, shared_ptr<ActionUpdateInfo> actions)
	{
		shared_ptr<TcpClientSession> currentSession = this->GetCurSession();
		int areaId = actions->areaid();
			
		const std::string & listenerAddress = actions->address();
		const std::string & queryAddress = currentSession->GetAddress();
		for (int index = 0; index < actions->action_names_size(); index++)
		{
			ActionProxyInfo actionInfo;
			actionInfo.mAreaId = actions->areaid();
			actionInfo.mQueryAddress = queryAddress;
			actionInfo.mListenerAddress = listenerAddress;
			actionInfo.mActionName = actions->action_names(index);
			this->AddActionInfo(actionInfo);
		}
		this->mAreaSessionMap.emplace(currentSession->GetAddress(), areaId);
		this->BroadCastActionList(areaId);
		return XCode::Successful;
	}

	XCode ActionRegisterManager::QueryActions(long long operId, shared_ptr<Int32Data> requestData, shared_ptr<ActionInfoList> returnData)
	{
		const int areaId = requestData->data();
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			ActionProxyInfo & actionInfo = this->mActionRegisterList[index];
			if (actionInfo.mAreaId == areaId || actionInfo.mAreaId == 0)
			{
				PB::ActionInfo * info = returnData->add_actionlist();
				info->set_adreid(actionInfo.mAreaId);
				info->set_actionname(actionInfo.mActionName);
				info->set_address(actionInfo.mListenerAddress);
			}
		}
		return XCode::Successful;
	}

	void ActionRegisterManager::BroadCastActionList(int areaId)
	{
		ActionInfoList returnData;
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			ActionProxyInfo & actionInfo = this->mActionRegisterList[index];
			if (actionInfo.mAreaId == areaId || actionInfo.mAreaId == 0)
			{
				PB::ActionInfo * info = returnData.add_actionlist();
				info->set_adreid(actionInfo.mAreaId);
				info->set_actionname(actionInfo.mActionName);
				info->set_address(actionInfo.mListenerAddress);
			}
		}
		for (auto iter = this->mAreaSessionMap.begin(); iter != this->mAreaSessionMap.end(); iter++)
		{
			if (iter->second == areaId)
			{
				const std::string & address = iter->first;				
				SharedTcpSession tcpSession = this->mNetWorkManager->GetTcpSession(address);
				if (tcpSession != nullptr)
				{
					RemoteScheduler callScheduler(tcpSession);
					callScheduler.Call("RemoteActionManager.UpdateActions", &returnData);
				}
			}			
		}
	}

	void ActionRegisterManager::AddActionInfo(ActionProxyInfo & actionInfo)
	{
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			if (this->mActionRegisterList[index] == actionInfo)
			{
				return;
			}
		}
		this->mActionRegisterList.push_back(actionInfo);
		SayNoDebugLog(actionInfo.mAreaId << "  " << actionInfo.mListenerAddress << "  " << actionInfo.mActionName);
		std::sort(this->mActionRegisterList.begin(), this->mActionRegisterList.end(), []
			(ActionProxyInfo & a1, ActionProxyInfo & a2)
		{
			return a1.mActionName.size() < a2.mActionName.size();
		});
	}
}
