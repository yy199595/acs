#include"ServiceRegistry.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<Core/TcpSessionListener.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/ActionScheduler.h>
#include<Coroutine/CoroutineManager.h>

#include<Other/ServiceNode.h>
#include<Service/ProxyService.h>

namespace SoEasy
{
	ServiceRegistry::ServiceRegistry()
	{

	}

	bool ServiceRegistry::OnInit()
	{
		this->mServiceIndex = 100;
		SayNoAssertRetFalse_F(LocalService::OnInit());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		REGISTER_FUNCTION_1(ServiceRegistry::RegisterNode, s2s::NodeRegister_Request);
		return true;
	}

	void ServiceRegistry::OnSystemUpdate()
	{
		LocalService::OnSystemUpdate();
	}

	void ServiceRegistry::OnInitComplete()
	{

	}

	XCode ServiceRegistry::RegisterNode(long long id, shared_ptr<s2s::NodeRegister_Request> nodeInfo)
	{
		const int areaId = nodeInfo->areaid();
		const int nodeId = nodeInfo->nodeid();
		const std::string & address = nodeInfo->address();
		SharedTcpSession tcpSession = this->GetCurTcpSession();
		long long key = (long long)areaId << 32 | nodeId;
		auto iter = this->mServiceNodeMap.find(key);
		if (iter != this->mServiceNodeMap.end())
		{
			return XCode::Failure;
		}
		ServiceNode * serviceNode = new ServiceNode(areaId, nodeId, address, tcpSession->GetAddress());
		for (int index = 0; index < nodeInfo->services_size(); index++)
		{
			serviceNode->AddService(nodeInfo->services(index));
		}
		SayNoDebugInfo(serviceNode->GetJsonString());
		this->mServiceNodeMap.emplace(key, serviceNode);
		return XCode::Successful;
	}
}
