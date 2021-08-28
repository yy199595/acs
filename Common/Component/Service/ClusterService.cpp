#include "ClusterService.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>

namespace Sentry
{
    bool ClusterService::Awake()
    {
        std::string ip;
        unsigned short port = 0;
        this->mAreaId = App::Get().GetConfig().GetAreaId();
        this->mNodeId = App::Get().GetConfig().GetNodeId();
		App::Get().GetConfig().GetValue("ListenAddress", "ip", ip);
		App::Get().GetConfig().GetValue("ListenAddress", "port", port);

        this->mListenAddress = ip + ":" + std::to_string(port);
        SayNoAssertRetFalse_F(this->mNodeComponent = gameObject->GetComponent<ServiceNodeComponent>());

        REGISTER_FUNCTION_1(ClusterService::DelNode, Int32Data);
        REGISTER_FUNCTION_1(ClusterService::AddNode, s2s::NodeData_NodeInfo);

        return LocalService::Awake();
    }

	void ClusterService::Start()
	{
		std::vector<Component *> components;
		s2s::NodeRegister_Request registerInfo;
		this->gameObject->GetComponents(components);

		for (Component * component : components)
		{
			ServiceBase * service = dynamic_cast<ServiceBase *>(component);
			if (service != nullptr)
			{
				registerInfo.add_services(service->GetTypeName());
			}
		}

		ServiceNode *centerNode = this->mNodeComponent->GetServiceNode(0);

		registerInfo.set_areaid(this->mAreaId);
		registerInfo.set_nodeid(this->mNodeId);
		registerInfo.set_address(this->mListenAddress);
		registerInfo.set_servername(App::Get().GetServerName());
		XCode code = centerNode->Invoke("ServiceRegistry", "RegisterNode", registerInfo);
		if (code != XCode::Successful)
		{
			SayNoDebugLog("register local service node fail");
			return;
		}
		SayNoDebugLog("register local service node successful");

	}

    XCode ClusterService::DelNode(long long, const Int32Data &serviceData)
    {
        const int nodeId = serviceData.data();
        bool res = this->mNodeComponent->DelNode(nodeId);
        return nodeId ? XCode::Successful : XCode::Failure;
    }

    XCode ClusterService::AddNode(long long, const s2s::NodeData_NodeInfo &nodeInfo)
    {
        const int nodeId = nodeInfo.nodeid();
        ServiceNode *serviceNode = this->mNodeComponent->GetServiceNode(nodeId);
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
		//this->mServiceManager->GetLocalServices(services);

		for (ServiceBase *service : services)
		{
			service->OnRefreshService();
		}

        return XCode::Successful;
    }
}// namespace Sentry