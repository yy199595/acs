#include "ClusterService.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Scene/SceneListenComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>

namespace Sentry
{
    bool ClusterService::Awake()
    {      
		__ADD_SERVICE_METHOD__(ClusterService::Add);
		__ADD_SERVICE_METHOD__(ClusterService::Del);
		ServerConfig & config = App::Get().GetConfig();
		this->mAreaId = App::Get().GetConfig().GetAreaId();
		this->mNodeId = App::Get().GetConfig().GetNodeId();
		SayNoAssertRetFalse_F(this->mNodeComponent = gameObject->GetComponent<ServiceNodeComponent>());
		return true;
    }

	void ClusterService::Start()
	{
		/*std::vector<Component *> components;
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
		SceneListenComponent * listenComponent = Scene::GetComponent<SceneListenComponent>();

		registerInfo.set_areaid(this->mAreaId);
		registerInfo.set_nodeid(this->mNodeId);
		registerInfo.set_address(listenComponent->GetAddress());
		registerInfo.set_servername(App::Get().GetServerName());
		XCode code = centerNode->Invoke("ServiceCenter", "Add", registerInfo);
		if (code != XCode::Successful)
		{
			SayNoDebugError("register local service node fail");
			return;
		}
		SayNoDebugLog("register all service to center successful");*/
	}

    XCode ClusterService::Del(long long, const Int32Data &serviceData)
    {
        const int nodeId = serviceData.data();
        bool res = this->mNodeComponent->DelNode(nodeId);
        return nodeId ? XCode::Successful : XCode::Failure;
    }

    XCode ClusterService::Add(long long, const s2s::NodeData_NodeInfo &nodeInfo)
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