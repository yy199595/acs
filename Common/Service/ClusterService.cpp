#include "ClusterService.h"
#include <Coroutine/CoroutineManager.h>
#include <Manager/ServiceManager.h>
#include <Manager/ServiceNodeManager.h>
#include <Other/ServiceNode.h>

namespace Sentry
{
    bool ClusterService::OnInit()
    {
        std::string ip;
        unsigned short port = 0;
        this->mAreaId = this->GetConfig().GetAreaId();
        this->mNodeId = this->GetConfig().GetNodeId();

        this->GetConfig().GetValue("ListenAddress", "ip", ip);
        this->GetConfig().GetValue("ListenAddress", "port", port);

        this->mListenAddress = ip + ":" + std::to_string(port);

        SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
        SayNoAssertRetFalse_F(this->mServiceNodeManager = this->GetManager<ServiceNodeManager>());

        REGISTER_FUNCTION_1(ClusterService::DelNode, Int32Data);
        REGISTER_FUNCTION_1(ClusterService::AddNode, s2s::NodeData_NodeInfo);

        return LocalService::OnInit();
    }

    void ClusterService::OnInitComplete()
    {
		/*std::vector<std::string> localServices;
		this->mServiceManager->GetLocalServices(localServices);
		ServiceNode *centerNode = this->mServiceNodeManager->GetServiceNode(0);
		SayNoAssertRet_F(centerNode && !localServices.empty());

		s2s::NodeRegister_Request registerInfo;
		registerInfo.set_areaid(this->mAreaId);
		registerInfo.set_nodeid(this->mNodeId);
		registerInfo.set_address(this->mListenAddress);
		registerInfo.set_servername(this->GetApp()->GetServerName());
		for (const std::string &name : localServices)
		{
			registerInfo.add_services()->assign(name);
		}
		XCode code = centerNode->Invoke("ServiceRegistry", "RegisterNode", registerInfo);
		if (code != XCode::Successful)
		{
			SayNoDebugLog("register local service node fail");
			return;
		}
		SayNoDebugLog("register local service node successful");*/

    }

    XCode ClusterService::DelNode(long long, const Int32Data &serviceData)
    {
        const int nodeId = serviceData.data();
        bool res = this->mServiceNodeManager->DelServiceNode(nodeId);
        return nodeId ? XCode::Successful : XCode::Failure;
    }

    XCode ClusterService::AddNode(long long, const s2s::NodeData_NodeInfo &nodeInfo)
    {
        const int nodeId = nodeInfo.nodeid();
        ServiceNode *serviceNode = this->mServiceNodeManager->GetServiceNode(nodeId);
        if (serviceNode == nullptr)
        {
            const int areaId = nodeInfo.areaid();
            const std::string &name = nodeInfo.servername();
            const std::string &address = nodeInfo.address();
            serviceNode = new ServiceNode(areaId, nodeId, name, address);
        }
        for (int index = 0; index < nodeInfo.services_size(); index++)
        {
            const std::string &service = nodeInfo.services(index);
            serviceNode->AddService(service);
        }
        // 通知所有服务
        std::vector<ServiceBase *> services;
        this->mServiceManager->GetLocalServices(services);

        for (ServiceBase *service : services)
        {
            service->OnRefreshService();
        }

        return XCode::Successful;
    }
}// namespace Sentry