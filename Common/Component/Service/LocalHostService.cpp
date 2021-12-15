#include "LocalHostService.h"
#include <Core/App.h>
#include <Service/RpcNode.h>
#include <Service/NodeProxyComponent.h>
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
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());
        return true;
    }

	void LocalHostService::OnStart()
    {
        std::vector<Component *> components;
        s2s::NodeRegister_Request registerInfo;
        this->gameObject->GetComponents(components);
        s2s::NodeInfo *nodeInfo = registerInfo.mutable_nodeinfo();

        nodeInfo->set_areaid(this->mAreaId);
        nodeInfo->set_nodeid(this->mNodeId);
        nodeInfo->set_servername(this->mNodeName);

        auto tcpServer = this->GetComponent<TcpServerComponent>();

        std::vector<const NetworkListener *> listeners;
        tcpServer->GetListeners(listeners);
        nodeInfo->set_serverip(tcpServer->GetHostIp());

        for (auto listener: listeners)
        {
            const auto &listenerConfig = listener->GetConfig();
            const std::string &name = listenerConfig.Name;
            const unsigned short port = listenerConfig.Port;
            nodeInfo->mutable_listeners()->insert({name, port});
        }

        for (Component *component: components)
        {
            if (auto *service = dynamic_cast<ServiceComponent *>(component))
            {
                nodeInfo->add_services(service->GetTypeName());
            }
        }

        RpcNode *centerNode = this->mNodeComponent->GetServiceNode(0);
        std::shared_ptr<s2s::NodeRegister_Response> response(new s2s::NodeRegister_Response());
        while(centerNode->Call("CenterHostService.Add", registerInfo, response) != XCode::Successful)
        {
            LOG_ERROR("register to center failure");
            App::Get().GetTaskComponent()->AwaitSleep(3000);
        }
        this->mToken = response->groupdata().token();
        this->mOpenTime = response->groupdata().opentime();
        this->mGroupName = response->groupdata().groupname();
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
        const int nodeId = serviceData.data();
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
            LOG_ERROR("remove node " << nodeProxy->GetNodeName());
        }
        return nodeId ? XCode::Successful : XCode::Failure;
    }

	XCode LocalHostService::Add(const s2s::NodeInfo & nodeInfo)
	{
		const int uid = nodeInfo.areaid() * 10000 + nodeInfo.nodeid();
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