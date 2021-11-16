#include "NodeServer.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>
#include <Service/NodeProxyComponent.h>
#include <Network/Listener/NetworkListener.h>
#include <Network/Listener/TcpServerComponent.h>
#include <Scene/RpcProtoComponent.h>
namespace GameKeeper
{
    bool NodeServer::Awake()
    {      
		__add_method(NodeServer::Add);
		__add_method(NodeServer::Del);
		GKAssertRetFalse_F(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());
		return true;
    }

	void NodeServer::Start()
	{
		std::vector<Component *> components;
		s2s::NodeRegister_Request registerInfo;
		this->gameObject->GetComponents(components);
        const ServerConfig & config = App::Get().GetConfig();

        s2s::NodeInfo * nodeInfo = registerInfo.mutable_nodeinfo();

        nodeInfo->set_areaid(config.GetAreaId());
        nodeInfo->set_nodeid(config.GetNodeId());
        nodeInfo->set_servername(config.GetNodeName());

        auto tcpServer = this->GetComponent<TcpServerComponent>();

        std::vector<const NetworkListener *> listeners;
        tcpServer->GetListeners(listeners);
        nodeInfo->set_serverip(tcpServer->GetHostIp());

        for(auto listener : listeners)
        {
            const auto &listenerConfig = listener->GetConfig();
            const std::string & name = listenerConfig.Name;
            const unsigned short port = listenerConfig.Port;
            nodeInfo->mutable_listeners()->insert({name, port});
        }

		for (Component * component : components)
		{
            if (auto *service = dynamic_cast<ServiceComponent *>(component))
            {
                nodeInfo->add_services(service->GetTypeName());
            }
        }
        XCode code = XCode::Failure;
        auto corComponent = App::Get().GetCorComponent();
        auto *listenComponent = this->GetComponent<TcpServerComponent>();
        RpcNodeProxy *centerNode = this->mNodeComponent->GetServiceNode(0);

        int count = 0;
        s2s::NodeRegister_Response responseInfo;
        while(code != XCode::Successful)
        {
            GKDebugInfo("start register self count = " << ++count);
            code = centerNode->Call("NodeCenter.Add", registerInfo, responseInfo);
            if (code != XCode::Successful)
            {
                GKDebugCode(code);
                corComponent->Sleep(3000);
                GKDebugError("register local service node fail " << code);
            }
        }
        this->mToken = responseInfo.groupdata().token();
        this->mOpenTime = responseInfo.groupdata().opentime();
        this->mGroupName = responseInfo.groupdata().groupname();
        GKDebugLog("register all service to center successful");
    }

    XCode NodeServer::Del(const Int32Data &serviceData)
    {
        const int nodeId = serviceData.data();
        bool res = this->mNodeComponent->DelNode(nodeId);
        return nodeId ? XCode::Successful : XCode::Failure;
    }

	XCode NodeServer::Add(const s2s::NodeInfo & nodeInfo)
	{
		const int uid = nodeInfo.areaid() * 10000 + nodeInfo.nodeid();
		RpcNodeProxy *serviceNode = this->mNodeComponent->GetServiceNode(uid);

        serviceNode->UpdateNodeProxy(nodeInfo);
		// 通知所有服务
		std::vector<Component *> components;
        this->GetComponents(components);
		for (Component * component : components)
		{
			if (auto serviceComponent = dynamic_cast<ServiceComponent*>(component))
			{
				serviceComponent->OnRefreshService();
			}
		}
		return XCode::Successful;
	}

}// namespace GameKeeper