#include "NodeCenter.h"
#include <Core/App.h>
#include <Util/StringHelper.h>
#include <Service/RpcNodeProxy.h>
#include <Scene/RpcComponent.h>
#include <Scene/RpcProtoComponent.h>
#include <Service/NodeProxyComponent.h>
#include <Util/FileHelper.h>
namespace GameKeeper
{
	bool NodeCenter::Awake()
    {
        __add_method(NodeCenter::Add);
        __add_method(NodeCenter::Query);
        GKAssertRetFalse_F(this->mRpcComponent = this->GetComponent<RpcComponent>());
        GKAssertRetFalse_F(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());
        return this->OnLoadConfig();
    }

    bool NodeCenter::OnLoadConfig()
    {
        this->mGroupNodeMap.clear();
        rapidjson::Document document;
        const std::string &path = App::Get().GetConfigPath();
        if (!FileHelper::ReadJsonFile(path + "group.json", document))
        {
            GKDebugError("not find config " << path + "group.json");
            return false;
        }

        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string name = iter->name.GetString();
            const rapidjson::Value &jsonData = iter->value;
            GKAssertRetFalse_F(jsonData.IsObject())
            GKAssertRetFalse_F(jsonData.HasMember("Id"));

            ServerGroupConfig groupNodeInfo;

            groupNodeInfo.mName = name;
            groupNodeInfo.mGroupId = jsonData["Id"].GetUint();
            groupNodeInfo.mToken = StringHelper::CreateNewToken();
            this->mGroupNodeMap.emplace(groupNodeInfo.mGroupId, groupNodeInfo);
        }
        return true;
    }


	XCode NodeCenter::Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response)
    {
        const s2s::NodeInfo & registerNodeInfo = nodeInfo.nodeinfo();
        const unsigned short areaId = registerNodeInfo.areaid();

        auto groupConfig = this->GetGroupConfig(areaId);
        if(groupConfig == nullptr)
        {
            return XCode::NotServerGroupConfig;
        }

        long long socketId = this->GetCurSocketId();
        unsigned short nodeId = registerNodeInfo.nodeid();
        unsigned int globalId = areaId * 10000 + nodeId;

        auto nodeProxy = this->mNodeComponent->Create(globalId);
        if (!nodeProxy->UpdateNodeProxy(nodeInfo.nodeinfo(), socketId))
        {
            return XCode::InitNodeProxyFailure;
        }

        this->AddNewNode(areaId, globalId);
        this->NoticeAllNode(registerNodeInfo);

        response.set_globalid(globalId);
        response.mutable_groupdata()->set_token(groupConfig->mToken);
        response.mutable_groupdata()->set_groupname(groupConfig->mName);
        response.mutable_groupdata()->set_groupid(groupConfig->mGroupId);
        return XCode::Successful;
    }

    void NodeCenter::AddNewNode(unsigned short areaId, unsigned int nodeId)
    {
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter == this->mServiceNodeMap.end())
        {
            std::set<unsigned int> temp;
            this->mServiceNodeMap.emplace(areaId, temp);
        }
        this->mServiceNodeMap[areaId].insert(nodeId);
    }

	XCode NodeCenter::Query(const s2s::NodeQuery_Request & request, s2s::NodeQuery_Response & response)
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

    void NodeCenter::NoticeAllNode(const s2s::NodeInfo & nodeInfo)
    {
        auto areaId = (unsigned short)nodeInfo.areaid();
        auto iter = this->mServiceNodeMap.find(areaId);
        if(iter != this->mServiceNodeMap.end())
        {
            for(unsigned int id : iter->second)
            {
               auto nodeProxy = this->mNodeComponent->GetServiceNode(id);
               nodeProxy->Notice("NodeServer.Add", nodeInfo);
            }
        }
    }

    const ServerGroupConfig *NodeCenter::GetGroupConfig(unsigned int groupId)
    {
        auto iter = this->mGroupNodeMap.find(groupId);
        return iter != this->mGroupNodeMap.end() ? &iter->second : nullptr;
    }
}
