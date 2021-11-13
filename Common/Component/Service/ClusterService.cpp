#include "ClusterService.h"

#include <Core/App.h>
#include <Service/NodeProxy.h>
#include <Service/NodeProxyComponent.h>
#include <Network/Listener/TcpServerComponent.h>
namespace GameKeeper
{
    bool ClusterService::Awake()
    {      
		__add_method(ClusterService::Add);
		__add_method(ClusterService::Del);
		ServerConfig & config = App::Get().GetConfig();
		this->mAreaId = App::Get().GetConfig().GetAreaId();
		this->mNodeId = App::Get().GetConfig().GetNodeId();
		GKAssertRetFalse_F(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());
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
		NodeProxy *centerNode = this->mNodeComponent->GetServiceNode(0);
		TcpServerComponent * listenComponent = this->GetComponent<TcpServerComponent>();

		registerInfo.set_areaid(this->mAreaId);
		registerInfo.set_nodeid(this->mNodeId);
		//TODO
		registerInfo.set_servername(App::Get().GetServerName());
		XCode code = centerNode->Call("CenterService.Add", registerInfo, responseInfo);
		if (code != XCode::Successful)
		{
			GKDebugError("register local service node fail");
			return;
		}
		GKDebugLog("register all service to center successful");

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
		NodeProxy *serviceNode = this->mNodeComponent->GetServiceNode(uid);
		if (serviceNode == nullptr)
        {
            const int areaId = uid / 10000;
            const int nodeId = uid % 10000;
            const std::string &name = nodeInfo.servername();
            const std::string &address = nodeInfo.address();
            serviceNode = new NodeProxy(areaId, nodeId, name, address);
        }

		for (int index = 0; index < nodeInfo.services_size(); index++)
		{
			const std::string &service = nodeInfo.services(index);
			if (serviceNode->AddService(service))
			{
				GKDebugLog(nodeInfo.servername() << " add new service " << service);
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

}// namespace GameKeeper