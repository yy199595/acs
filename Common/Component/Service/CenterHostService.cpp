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
        const std::string &path = App::Get().GetServerPath().GetConfigPath();
        LOG_CHECK_RET_FALSE(this->LoadHostConfig(path + "host.json"));
        LOG_CHECK_RET_FALSE(this->LoadGroupConfig(path + "group.json"));
        return true;
    }

	XCode CenterHostService::Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response)
    {
        const s2s::NodeInfo & registerNodeInfo = nodeInfo.node_info();
        const unsigned short areaId = registerNodeInfo.area_id();

        auto groupConfig = this->GetGroupConfig(areaId);
        if(groupConfig == nullptr)
        {
            return XCode::NotServerGroupConfig;
        }

        int nodeId = registerNodeInfo.node_id();
        const int globalId = areaId * 10000 + nodeId;

        auto nodeProxy = this->mNodeComponent->Create(globalId);
        if (!nodeProxy->UpdateNodeProxy(nodeInfo.node_info()))
        {
            return XCode::InitNodeProxyFailure;
        }

        response.set_node_id(globalId);
        this->AddNewNode(areaId, globalId);
        response.mutable_group_data()->set_token(groupConfig->mToken);
        response.mutable_group_data()->set_group_name(groupConfig->mName);
        response.mutable_group_data()->set_group_id(groupConfig->mGroupId);
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
                XCode code = rpcNode->Call("LocalHostService.Add", nodeInfo, taskSource);
                if(code != XCode::Successful)
                {
                    return code;
                }
                LOG_DEBUG("add rpc node to {0}", nodeInfo.server_name());
            }
        }
        return XCode::Successful;
    }

    const ServerGroupConfig *CenterHostService::GetGroupConfig(unsigned int groupId)
    {
        auto iter = this->mGroupNodeMap.find(groupId);
        return iter != this->mGroupNodeMap.end() ? &iter->second : nullptr;
    }

    bool CenterHostService::LoadHostConfig(const std::string &path)
    {
        std::string md5;
        rapidjson::Document document;
        if (!Helper::File::ReadJsonFile(path, document, this->mHostConfigMd5))
        {
            LOG_ERROR("not find config {0}", path);
            return false;
        }
        if(this->mHostConfigMd5 == md5)
        {
            return true;
        }
        this->mHostConfigMd5 = md5;
        this->mServiceHosts.clear();
        LOG_CHECK_RET_FALSE(document.HasMember("host") && document["host"].IsArray());

        for (size_t index = 0; index < document["host"].Size(); index++)
        {
            std::string host = document["host"][index].GetString();
            this->mServiceHosts.emplace(host);
        }
        return true;
    }

    bool CenterHostService::LoadGroupConfig(const std::string &path)
    {
        std::string md5;
        rapidjson::Document  document;
        if (!Helper::File::ReadJsonFile(path, document, md5))
        {
            LOG_ERROR("not find config {0}", path);
            return false;
        }
        if(this->mGroupConfigMd5 == md5)
        {
            return true;
        }
        this->mGroupConfigMd5 = md5;
        this->mGroupNodeMap.clear();
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const std::string name = iter->name.GetString();
            const rapidjson::Value &jsonData = iter->value;
            LOG_CHECK_RET_FALSE(jsonData.IsObject())
            LOG_CHECK_RET_FALSE(jsonData.HasMember("Id"));

            ServerGroupConfig groupNodeInfo;

            groupNodeInfo.mName = name;
            groupNodeInfo.mGroupId = jsonData["Id"].GetUint();
            groupNodeInfo.mToken = Helper::String::CreateNewToken();
            this->mGroupNodeMap.emplace(groupNodeInfo.mGroupId, groupNodeInfo);
        }
        return true;
    }
}
