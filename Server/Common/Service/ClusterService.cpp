#include"ClusterService.h"
#include<Manager/ServiceManager.h>
#include<NetWork/ActionScheduler.h>
#include<Manager/ServiceNodeManager.h>
#include<Coroutine/CoroutineManager.h>
#include<Service/ProxyService.h>
#include<Other/ServiceNode.h>
namespace SoEasy
{
	bool ClusterService::OnInit()
	{
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("AreaId", this->mAreaId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("NodeId", this->mNodeId));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("QueryAddress", mQueryAddress));
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", mListenAddress));
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		SayNoAssertRetFalse_F(this->mServiceNodeManager = this->GetManager<ServiceNodeManager>());

		REGISTER_FUNCTION_1(ClusterService::RemoveService, Int32Data);
		return LocalService::OnInit();
	}

	void ClusterService::OnInitComplete()
	{
		this->StarRegister();
	}

	void ClusterService::OnConnectDone(SharedTcpSession tcpSession)
	{

	}

	void ClusterService::StarRegister()
	{
		std::vector<ServiceBase*> localServices;
		this->mServiceManager->GetLocalServices(localServices);
		ServiceNode * centerNode = this->mServiceNodeManager->GetServiceNode(0);
		SayNoAssertRet_F(centerNode && !localServices.empty());

		s2s::NodeRegister_Request registerInfo;
		registerInfo.set_areaid(this->mAreaId);
		registerInfo.set_nodeid(this->mNodeId);
		registerInfo.set_address(this->mListenAddress);
		registerInfo.set_servername(this->GetApp()->GetServerName());
		for (ServiceBase * localService : localServices)
		{
			const std::string & name = localService->GetServiceName();
			registerInfo.add_services()->assign(name);
		}
		XCode code = centerNode->Call("ServiceRegistry", "RegisterNode", &registerInfo);
		SayNoDebugLog("register local service successful");
	}

	XCode ClusterService::RemoveService(long long, shared_ptr<Int32Data> serviceData)
	{
		const int serviceId = serviceData->data();
		return XCode::Successful;
	}
}