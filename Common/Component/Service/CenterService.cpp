#include "CenterService.h"
#include <Core/App.h>
#include <Util/StringHelper.h>
#include <Service/RpcNodeProxy.h>
#include <Scene/RpcComponent.h>
#include <Scene/RpcProtoComponent.h>
#include <Service/NodeProxyComponent.h>
namespace GameKeeper
{
	bool CenterService::Awake()
	{
		__add_method(CenterService::Add);
		__add_method(CenterService::Query);
        this->mNodeComponent = this->GetComponent<NodeProxyComponent>();
		return true;
	}

    void CenterService::Start()
    {

    }

	XCode CenterService::Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response)
    {
        const unsigned short areaId = nodeInfo.nodeinfo().areaid();
        const unsigned short nodeId = nodeInfo.nodeinfo().nodeid();
        unsigned int globalId = (unsigned int) areaId << 32 | nodeId;

        long long socketId = this->GetCurSocketId();
        auto nodeProxy = this->mNodeComponent->Create(globalId);
        if (!nodeProxy->UpdateNodeProxy(nodeInfo.nodeinfo(), socketId))
        {
            return XCode::Failure;
        }
        this->NoticeAllNode(nodeInfo.nodeinfo());
        this->AddNewNode(areaId, globalId);
        return XCode::Successful;
    }

    void CenterService::AddNewNode(unsigned short areaId, unsigned int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter == this->mServiceNodeMap.end())
        {
            std::set<unsigned int> temp;
            this->mServiceNodeMap.emplace(areaId, temp);
        }
        this->mServiceNodeMap[areaId].insert(nodeId);
    }

	XCode CenterService::Query(const s2s::NodeQuery_Request & request, s2s::NodeQuery_Response & response)
	{
        auto areaId = (unsigned short)request.areaid();
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
               auto nodeInfo = response.add_nodeinfos();
               nodeInfo->CopyFrom(nodeProxy->GetNodeInfo());
            }
        }
        return XCode::Successful;
	}

    void CenterService::NoticeAllNode(const s2s::NodeInfo & nodeInfo)
    {
        auto areaId = (unsigned short)nodeInfo.areaid();
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter != this->mServiceNodeMap.end())
        {
            for(unsigned int id : iter->second)
            {
               auto nodeProxy = this->mNodeComponent->GetServiceNode(id);
               nodeProxy->Notice("ClusterService.Add", nodeInfo);
            }
        }
    }
}
