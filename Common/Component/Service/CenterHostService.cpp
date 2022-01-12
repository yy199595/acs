#include"CenterHostService.h"
#include<Core/App.h>
#include<Util/StringHelper.h>
#include<Service/RpcNode.h>
#include<Util/FileHelper.h>
#include<Rpc/RpcClientComponent.h>
#include"Component/Scene/RpcNodeComponent.h"


namespace GameKeeper
{
	bool CenterHostService::Awake()
    {
        BIND_RPC_FUNCTION(CenterHostService::Add);
        BIND_RPC_FUNCTION(CenterHostService::Query);
        BIND_RPC_FUNCTION(CenterHostService::QueryHosts);
        return this->OnLoadConfig();
    }

    bool CenterHostService::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<RpcNodeComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcClientComponent>());
        return true;
    }

    bool CenterHostService::OnLoadConfig()
    {
        return true;
    }

	XCode CenterHostService::Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response)
    {
        const s2s::NodeInfo & registerNodeInfo = nodeInfo.node_info();
        const unsigned short areaId = registerNodeInfo.area_id();

        auto nodeProxy = this->mNodeComponent->Create(registerNodeInfo.node_id());
        if (!nodeProxy->UpdateNodeProxy(nodeInfo.node_info()))
        {
            return XCode::InitNodeProxyFailure;
        }

        response.set_node_id(registerNodeInfo.node_id());
        this->AddNewNode(areaId, registerNodeInfo.node_id());
        return this->NoticeAllNode(registerNodeInfo);
    }

    XCode CenterHostService::QueryHosts(s2s::HostQuery_Response &response)
    {
        auto iter = this->mServiceHosts.begin();
        for(; iter != this->mServiceHosts.end(); iter++)
        {
            response.add_hosts(*iter);
        }
        return XCode::Successful;
    }

    void CenterHostService::AddNewNode(unsigned short areaId, unsigned int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter == this->mServiceNodeMap.end())
        {
            std::set<unsigned int> temp;
            this->mServiceNodeMap.emplace(areaId, temp);
        }
        this->mServiceNodeMap[areaId].insert(nodeId);
    }

	XCode CenterHostService::Query(const s2s::NodeQuery_Request & request, s2s::NodeQuery_Response & response)
	{
        auto areaId = (unsigned short)request.area_id();
        const std::string & service = request.service();
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter == this->mServiceNodeMap.end())
        {
            return XCode::Failure;
        }
        for(unsigned int id : iter->second)
        {
            auto nodeProxy = this->mNodeComponent->GetServiceNode(id);
            if(nodeProxy!= nullptr && nodeProxy->HasService(service))
            {
               auto nodeInfo = response.add_node_infos();
               nodeInfo->CopyFrom(nodeProxy->GetNodeInfo());
            }
        }
        return XCode::Successful;
	}

    XCode CenterHostService::NoticeAllNode(const s2s::NodeInfo & nodeInfo)
    {
        auto areaId = (unsigned short)nodeInfo.area_id();
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter != this->mServiceNodeMap.end())
        {
            for(unsigned int id : iter->second)
            {
                std::shared_ptr<RpcTaskSource> taskSource(new RpcTaskSource());
                RpcNode * rpcNode = this->mNodeComponent->GetServiceNode(id);
                if(rpcNode->Call("LocalHostService.Add", nodeInfo, taskSource) == XCode::Successful)
                {
                    LOG_DEBUG("add rpc node to ", nodeInfo.server_name());
                }
            }
        }
        return XCode::Successful;
    }
}
