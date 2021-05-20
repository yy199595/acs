#include"ServiceRegistry.h"
#include<Core/Applocation.h>
#include<Core/TcpSessionListener.h>
#include<Util/StringHelper.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/RemoteScheduler.h>
#include<Protocol/ServerCommon.pb.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	bool ServiceRegistry::OnInit()
	{
		REGISTER_FUNCTION_1(ServiceRegistry::RegisterActions, ActionUpdateInfo);
		REGISTER_FUNCTION_2(ServiceRegistry::QueryActions, Int32Data, ActionInfoList);
		return true;
	}

	void ServiceRegistry::OnInitComplete()
	{
		
	}

	XCode ServiceRegistry::RegisterActions(long long operId, shared_ptr<ActionUpdateInfo> actions)
	{
		int areaId = actions->areaid();			
		const std::string & listenerAddress = actions->address();
		for (int index = 0; index < actions->action_names_size(); index++)
		{
			ActionProxyInfo actionInfo;
			actionInfo.mAreaId = actions->areaid();
			actionInfo.mListenerAddress = listenerAddress;
			actionInfo.mActionName = actions->action_names(index);
			this->AddActionInfo(actionInfo);
		}
		return XCode::Successful;
	}

	XCode ServiceRegistry::QueryActions(long long operId, shared_ptr<Int32Data> requestData, shared_ptr<ActionInfoList> returnData)
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

	void ServiceRegistry::AddActionInfo(ActionProxyInfo & actionInfo)
	{
		for (size_t index = 0; index < this->mActionRegisterList.size(); index++)
		{
			if (this->mActionRegisterList[index] == actionInfo)
			{
				return;
			}
		}
		this->mActionRegisterList.push_back(actionInfo);
		SayNoDebugInfo("********** register service " << actionInfo.mAreaId 
			<< "  " << actionInfo.mListenerAddress << "  " << actionInfo.mActionName);

		std::sort(this->mActionRegisterList.begin(), this->mActionRegisterList.end(), []
		(ActionProxyInfo & a1, ActionProxyInfo & a2)
		{
			return a1.mActionName.size() < a2.mActionName.size();
		});
	}
}
