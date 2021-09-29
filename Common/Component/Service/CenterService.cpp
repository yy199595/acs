#include "CenterService.h"
#include <Core/App.h>
#include <Util/StringHelper.h>
#include <Service/ServiceNode.h>

namespace Sentry
{
    CenterService::CenterService()
    {
    }

	bool CenterService::Awake()
	{
		__add_method(CenterService::Add);
		__add_method(CenterService::Query);
		return LocalServiceComponent::Awake();
	}

    void CenterService::Start()
    {

    }

    XCode CenterService::Add(const s2s::NodeRegister_Request &nodeInfo)
    {
        const int areaId = nodeInfo.areaid();
        const int nodeId = nodeInfo.nodeid();

        const std::string &address = nodeInfo.address();
        const std::string &nodeName = nodeInfo.servername();
		if (address.empty() || nodeName.empty())
		{
			return XCode::Failure;
		}
        //SharedTcpSession tcpSession = this->GetCurTcpSession();
        long long key = (long long) areaId << 32 | nodeId;
        auto iter = this->mServiceNodeMap.find(key);
        if (iter != this->mServiceNodeMap.end())
        {
            return XCode::Failure;
        }
        ServiceNode *serviceNode = new ServiceNode(areaId, nodeId, nodeName, address);
        for (int index = 0; index < nodeInfo.services_size(); index++)
        {
            serviceNode->AddService(nodeInfo.services(index));
        }
        this->mServiceNodeMap.emplace(key, serviceNode);
		SayNoDebugLog(nodeName << " [" << address << "] register successful ......");

        //this->NoticeNode(areaId);
        return XCode::Successful;
    }

    XCode CenterService::Query(const com::Int32Data &areaData, s2s::NodeData_Array &nodeArray)
    {
        const int areaId = areaData.data();
        auto iter = this->mServiceNodeMap.begin();
        for (; iter != this->mServiceNodeMap.end(); iter++)
        {
            ServiceNode *serviceNode = iter->second;
            if (serviceNode->GetAreaId() == areaId)
            {
                s2s::NodeData_NodeInfo *nodeData = nodeArray.add_nodearray();
                const s2s::NodeData_NodeInfo &nodeInfo = serviceNode->GetNodeMessage();
                nodeData->CopyFrom(nodeInfo);
            }
        }
        return XCode::Successful;
    }

    void CenterService::NoticeNode(int areaId)
    {
        auto iter = this->mServiceNodeMap.begin();
        for (; iter != this->mServiceNodeMap.end(); iter++)
        {
            ServiceNode *serviceNode = iter->second;
            // 通知同一组服务器节点和公共服务器节点
            if (serviceNode->GetAreaId() == areaId || serviceNode->GetAreaId() == 0)
            {
                const s2s::NodeData_NodeInfo &nodeInfo = serviceNode->GetNodeMessage();
                serviceNode->Notice("ClusterService", "AddNode", nodeInfo);
            }
        }
    }
}
