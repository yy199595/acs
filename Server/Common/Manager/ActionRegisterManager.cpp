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
		REGISTER_FUNCTION_2(ActionRegisterManager::QueryActions, Int32Data, ActionInfoList);

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

	XCode ActionRegisterManager::RegisterActions(long long operId, shared_ptr<ActionUpdateInfo> actions)
	{
		shared_ptr<TcpClientSession> currentSession = this->GetCurSession();
		int areaId = actions->areaid();
		const string & address = actions->address();
		
		for (int index = 0; index < actions->action_names_size(); index++)
		{
			ActionInfo actionInfo;
			actionInfo.mAreaId = actions->areaid();
			actionInfo.mAddress = actions->address();
			actionInfo.mOperId = currentSession->GetSocketId();
			actionInfo.mActionName = actions->action_names(index);
			this->AddActionInfo(actionInfo);
		}
		return XCode::Successful;
	}

	XCode ActionRegisterManager::QueryActions(long long operId, shared_ptr<Int32Data> requestData, shared_ptr<ActionInfoList> returnData)
	{
		const int areaId = requestData->data();
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			ActionInfo & actionInfo = this->mActionRegisterList[index];
			if (actionInfo.mAreaId == areaId || actionInfo.mAreaId == 0)
			{
				PB::ActionInfo * info = returnData->add_actionlist();
				info->set_adreid(actionInfo.mAreaId);
				info->set_address(actionInfo.mAddress);
				info->set_actionname(actionInfo.mActionName);
			}
		}
		return XCode::Successful;
	}

	void ActionRegisterManager::AddActionInfo(ActionInfo & actionInfo)
	{
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			if (this->mActionRegisterList[index] == actionInfo)
			{
				return;
			}
		}
		this->mActionRegisterList.push_back(actionInfo);
		SayNoDebugLog(actionInfo.mAreaId << "  " << actionInfo.mAddress << "  " << actionInfo.mActionName);
		std::sort(this->mActionRegisterList.begin(), this->mActionRegisterList.end(), []
			(ActionInfo & a1, ActionInfo & a2)
		{
			return a1.mActionName.size() < a2.mActionName.size();
		});
	}
}
