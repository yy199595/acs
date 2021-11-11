#include "CenterService.h"
#include <Core/App.h>
#include <Util/StringHelper.h>
#include <Service/NodeProxy.h>
#include <Scene/RpcProtoComponent.h>
namespace GameKeeper
{
	bool CenterService::Awake()
	{
		__add_method(CenterService::Add);
		__add_method(CenterService::Query);
		return true;
	}

    void CenterService::Start()
    {

    }

	XCode CenterService::Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response)
	{
		const int areaId = nodeInfo.areaid();
		const int nodeId = nodeInfo.nodeid();
		const std::string &address = nodeInfo.address();
		const std::string &nodeName = nodeInfo.servername();
		if (address.empty() || nodeName.empty())
		{
			return XCode::Failure;
		}

        auto serviceNode = new NodeProxy(areaId, nodeId, nodeName, address);

        const int key = serviceNode->GetNodeUId();
		auto iter = this->mServiceNodeMap.find(key);
		if (iter != this->mServiceNodeMap.end())
		{
            delete serviceNode;
			return XCode::Failure;
		}
		auto protoComponent = this->GetComponent<RpcProtoComponent>();
		for (int index = 0; index < nodeInfo.services_size(); index++)
		{
			const std::string & service = nodeInfo.services(index);
			if (!protoComponent->HasService(service))
			{
				GKDebugError("register [ " << service << " ] failure");
				return XCode::Failure;
			}
			serviceNode->AddService(service);
			GKDebugLog(nodeName << " add new service [ " << service << " ]");
		}
		response.set_uid(key);
		this->mServiceNodeMap.emplace(key, serviceNode);
		GKDebugLog(nodeName << " [" << address << "] register successful ......");

		return XCode::Successful;
	}

	XCode CenterService::Query(const s2s::NodeQuery_Request & request, s2s::NodeQuery_Response & response)
	{
        const int areaId = request.areaid();
        const std::string & service = request.servicename();

        auto iter = this->mServiceNodeMap.begin();
        for(; iter != this->mServiceNodeMap.end(); iter++)
        {
            NodeProxy * node = iter->second;
            if(node->GetAreaId() == areaId && node->HasService(service))
            {
               s2s::NodeInfo * nodeInfo = response.add_nodeinfos();
                nodeInfo->set_address(node->GetAddress());
            }
        }
		return XCode::Successful;
	}

    void CenterService::NoticeAllNode(const s2s::NodeInfo & nodeInfo)
    {

    }
}
