#include "LocalHostService.h"
#include <Core/App.h>
#include <Service/RpcNode.h>
#include "Component/Scene/RpcNodeComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Scene/OperatorComponent.h"
#include<Scene/RpcConfigComponent.h>
namespace GameKeeper
{
    bool LocalHostService::Awake()
    {
		BIND_RPC_FUNCTION(LocalHostService::Add);
		BIND_RPC_FUNCTION(LocalHostService::Del);
        BIND_RPC_FUNCTION(LocalHostService::Hotfix);
        BIND_RPC_FUNCTION(LocalHostService::ReLoadConfig);
		LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("AreaId", this->mAreaId));
		LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("NodeId", this->mNodeId));
		LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("NodeName", this->mNodeName));
		return true;
    }
    bool LocalHostService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<RpcNodeComponent>());
        return true;
    }

	void LocalHostService::OnStart()
    {
        std::vector<Component *> components;
        s2s::NodeRegister_Request registerInfo;
        this->gameObject->GetComponents(components);
        s2s::NodeInfo *nodeInfo = registerInfo.mutable_node_info();

        nodeInfo->set_area_id(this->mAreaId);
        nodeInfo->set_node_id(this->mNodeId);
        nodeInfo->set_server_name(this->mNodeName);

        auto tcpServer = this->GetComponent<TcpServerComponent>();

        std::vector<const NetworkListener *> listeners;
        tcpServer->GetListeners(listeners);
        nodeInfo->set_server_ip(tcpServer->GetHostIp());

        for (auto listener: listeners) {
            const auto &listenerConfig = listener->GetConfig();
            const std::string &name = listenerConfig.Name;
            const unsigned short port = listenerConfig.Port;
            nodeInfo->mutable_listeners()->insert({name, port});
        }

        for (Component *component: components) {
            if (auto *service = dynamic_cast<ServiceComponent *>(component)) {
                nodeInfo->add_services(service->GetTypeName());
            }
        }
        std::shared_ptr<RpcTaskSource> taskSource(new RpcTaskSource());
        auto rpcNode = this->mNodeComponent->GetServiceNode(0);
        XCode code = rpcNode->Call("CenterHostService.Add", registerInfo, taskSource);

        auto response = taskSource->GetData<s2s::NodeRegister_Response>();
        if (response == nullptr) {
            LOG_ERROR("register to center failure");
        }
        this->mToken = response->group_data().token();
        this->mOpenTime = response->group_data().open_time();
        this->mGroupName = response->group_data().group_name();
        LOG_DEBUG("register all service to center successful");
    }

    XCode LocalHostService::Hotfix()
    {
        auto operComponent = this->GetComponent<OperatorComponent>();
        if(operComponent != nullptr)
        {
            operComponent->StartHotfix();
        }
		return XCode::Successful;
    }

    XCode LocalHostService::ReLoadConfig()
    {
        auto operComponent = this->GetComponent<OperatorComponent>();
        if(operComponent != nullptr)
        {
            if(!operComponent->StartLoadConfig())
            {
                return XCode::Failure;
            }
        }
        return XCode::Successful;
    }

    XCode LocalHostService::Del(const Int32Data &serviceData)
    {
        const int nodeId = serviceData.value();
        RpcNode *nodeProxy = this->mNodeComponent->GetServiceNode(nodeId);
        if(nodeProxy != nullptr)
        {
            std::vector<Component *> components;
            this->GetComponents(components);
            for (Component * component : components)
            {
                if (auto serviceComponent = dynamic_cast<INodeRefresh*>(component))
                {
                    serviceComponent->OnDelRpcNode(nodeProxy);
                }
            }
            LOG_ERROR("remove node {0}", nodeProxy->GetNodeName());
        }
        return nodeId ? XCode::Successful : XCode::Failure;
    }

	XCode LocalHostService::Add(const s2s::NodeInfo & nodeInfo)
	{
		const int uid = nodeInfo.area_id() * 10000 + nodeInfo.node_id();
		RpcNode *serviceNode = this->mNodeComponent->GetServiceNode(uid);

        serviceNode->UpdateNodeProxy(nodeInfo);
		// 通知所有服务
		std::vector<Component *> components;
        this->GetComponents(components);
		for (Component * component : components)
		{
			if (auto serviceComponent = dynamic_cast<INodeRefresh*>(component))
			{
                serviceComponent->OnAddRpcNode(serviceNode);
			}
		}
		return XCode::Successful;
	}

}// namespace GameKeeper