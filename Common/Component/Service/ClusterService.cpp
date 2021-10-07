#include "ClusterService.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Scene/ListenerComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>
namespace Sentry
{
    bool ClusterService::Awake()
    {      
		__add_method(ClusterService::Add);
		__add_method(ClusterService::Del);
		ServerConfig & config = App::Get().GetConfig();
		this->mAreaId = App::Get().GetConfig().GetAreaId();
		this->mNodeId = App::Get().GetConfig().GetNodeId();
		SayNoAssertRetFalse_F(this->mNodeComponent = this->GetComponent<ServiceNodeComponent>());
		return true;
    }

	void ClusterService::Start()
	{
		std::vector<Component *> components;
		s2s::NodeRegister_Request registerInfo;
		this->gameObject->GetComponents(components);

		for (Component * component : components)
		{
            if (ServiceComponent *service = dynamic_cast<ServiceComponent *>(component))
            {
                registerInfo.add_services(service->GetTypeName());
            }
        }

		s2s::NodeRegister_Response responseInfo;
		ServiceNode *centerNode = this->mNodeComponent->GetServiceNode(0);
		ListenerComponent * listenComponent = this->GetComponent<ListenerComponent>();

		registerInfo.set_areaid(this->mAreaId);
		registerInfo.set_nodeid(this->mNodeId);
		registerInfo.set_address(listenComponent->GetAddress());
		registerInfo.set_servername(App::Get().GetServerName());
		XCode code = centerNode->Call("CenterService", "Add", registerInfo, responseInfo);
		if (code != XCode::Successful)
		{
			SayNoDebugError("register local service node fail");
			return;
		}
		SayNoDebugLog("register all service to center successful");

	}

    XCode ClusterService::Del(const Int32Data &serviceData)
    {
        const int nodeId = serviceData.data();
        bool res = this->mNodeComponent->DelNode(nodeId);
        return nodeId ? XCode::Successful : XCode::Failure;
    }

	XCode ClusterService::Add(const s2s::NodeInfo & nodeInfo)
	{
		const int uid = nodeInfo.uid();
		ServiceNode *serviceNode = this->mNodeComponent->GetServiceNode(uid);
		if (serviceNode == nullptr)
        {
            const int areaId = uid / 10000;
            const int nodeId = uid % 10000;
            const std::string &name = nodeInfo.servername();
            const std::string &address = nodeInfo.address();
            serviceNode = new ServiceNode(areaId, nodeId, name, address);
        }

		for (int index = 0; index < nodeInfo.services_size(); index++)
		{
			const std::string &service = nodeInfo.services(index);
			if (serviceNode->AddService(service))
			{
				SayNoDebugLog(nodeInfo.servername() << " add new service " << service);
			}
		}
		// 通知所有服务
		std::vector<Component *> components;
		this->gameObject->GetComponents(components);
		for (Component * component : components)
		{
			if (ServiceComponent * serviceComponent = dynamic_cast<ServiceComponent*>(component))
			{
				serviceComponent->OnRefreshService();
			}
		}
		return XCode::Successful;
	}

}// namespace Sentry