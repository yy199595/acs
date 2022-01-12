#include "LocalHostService.h"
#include "Core/App.h"
#include "Service/RpcNode.h"
#include "Component/Scene/RpcNodeComponent.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Scene/OperatorComponent.h"
#include"Component/Scene/RpcConfigComponent.h"
#include"Util/JsonHelper.h"
#include"RedisComponent.h"
namespace GameKeeper
{
    bool LocalHostService::Awake()
    {
        BIND_RPC_FUNCTION(LocalHostService::Add);
        BIND_RPC_FUNCTION(LocalHostService::Del);
        BIND_RPC_FUNCTION(LocalHostService::Hotfix);
        BIND_RPC_FUNCTION(LocalHostService::ReLoadConfig);
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("area_id", this->mAreaId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_id", this->mNodeId));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("node_name", this->mNodeName));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("center_server", "ip", this->mCenterIp));
        LOG_CHECK_RET_FALSE(App::Get().GetConfig().GetValue("center_server", "port", this->mCenterPort));
        return true;
    }
    bool LocalHostService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<RpcNodeComponent>());
        RpcNode * centerNode = this->mNodeComponent->CreateNode(
                0, this->mNodeName, this->mCenterIp, this->mCenterPort);
        return true;
    }

	void LocalHostService::OnStart()
    {
        std::vector<const NetworkListener *> listeners;
        auto redisComponent = this->GetComponent<RedisComponent>();
        auto tcpServerComponent = this->GetComponent<TcpServerComponent>();

        const NetworkListener *rpcListener = tcpServerComponent->GetListener("rpc");
        if (rpcListener == nullptr)
        {
            return;
        }
        RapidJsonWriter jsonWriter;
        const std::string &address = rpcListener->GetConfig().mAddress;
        jsonWriter.Add("time", Helper::Time::GetSecTimeStamp());
        jsonWriter.StartArray("service");
        std::vector<Component *> components;
        for (Component *component: components)
        {
            if (auto *service = dynamic_cast<ServiceComponent *>(component))
            {
                const std::string &name = component->GetTypeName();

            }
        }
        jsonWriter.EndArray();
    }

    void LocalHostService::OnLoadData()
    {

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
		RpcNode *serviceNode = this->mNodeComponent->GetServiceNode(nodeInfo.node_id());
        if(serviceNode == nullptr)
        {
            serviceNode = this->mNodeComponent->CreateNode(nodeInfo);
        }
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