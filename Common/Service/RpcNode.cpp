#include"RpcNode.h"
#include<Core/App.h>
#include<Coroutine/TaskComponent.h>
#include"Async/RpcTask/RpcTaskSource.h"
#include<Rpc/RpcClientComponent.h>
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include"Rpc/RpcComponent.h"
namespace GameKeeper
{
	RpcNode::RpcNode(int id)
		: mGlobalId(id), mIsClose(false), mSocketId(0)
	{
        this->mRpcComponent = App::Get().GetComponent<RpcComponent>();
        this->mRpcClientComponent = App::Get().GetComponent<RpcClientComponent>();
        LOG_CHECK_RET(this->mCorComponent = App::Get().GetComponent<TaskComponent>());
        LOG_CHECK_RET(this->mRpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>());
	}

	bool RpcNode::AddService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
			return true;
		}
		return false;
	}

    bool RpcNode::UpdateNodeProxy(const s2s::NodeInfo &nodeInfo)
    {
        auto iter = nodeInfo.listeners().find("rpc");
        if(iter == nodeInfo.listeners().end())
        {
            return false;
        }

        this->mServiceArray.clear();
        this->mAreaId = nodeInfo.area_id();
        this->mNodeId = nodeInfo.node_id();
        this->mNodeIp = nodeInfo.server_ip();
        this->mNodeName = nodeInfo.server_name();
        this->mNodePort = (unsigned short)iter->second;
        for(const std::string & name : nodeInfo.services())
        {
            this->mServiceArray.insert(name);
        }
        this->mNodeInfo.CopyFrom(nodeInfo);
        return true;
    }

    void RpcNode::ConnectToNode()
    {
        auto rpcClient = this->mRpcClientComponent->GetRpcSession(this->mSocketId);
        if(rpcClient == nullptr)
        {
            rpcClient = this->mRpcClientComponent->NewSession(this->mNodeName);
        }
        if(!rpcClient->IsOpen())
        {
            this->mSocketId = rpcClient->GetSocketProxy()->GetSocketId();
            auto method = NewMethodProxy(&RpcNode::OnConnectAfter, this);
            rpcClient->StartConnect(this->mNodeIp, this->mNodePort, method);
        }
    }

    void RpcNode::OnConnectAfter()
    {
        while(!this->mWaitSendQueue.empty())
        {
            auto message  = this->mWaitSendQueue.front();
            if(!this->mRpcClientComponent->Send(this->mSocketId, message))
            {
                break;
            }
            this->mWaitSendQueue.pop();
        }
    }

    void RpcNode::Destory()
    {
        size_t size = this->mWaitSendQueue.size();
        this->mRpcClientComponent->CloseSession(this->mSocketId);
        if(size > 0)
        {
            LOG_ERROR("send queue has " << size << " data");
        }
        delete this;
    }

    bool RpcNode::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

    void RpcNode::GetServices(std::vector<std::string> & services)
    {
        services.clear();
        for(const std::string & name : this->mServiceArray)
        {
            services.emplace_back(name);
        }
    }

	std::shared_ptr<com::Rpc_Request> RpcNode::NewRequest(const std::string & method)
	{
		auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
		if (config == nullptr)
		{
			return nullptr;
		}
        std::shared_ptr<com::Rpc_Request> request(new com::Rpc_Request());
		request->set_method_id(config->MethodId);
        if(!this->mRpcClientComponent->Send(this->mSocketId, request))
        {
            this->ConnectToNode();
            this->mWaitSendQueue.push(request);
        }
        return request;
	}

    XCode RpcNode::Call(const string &func, std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("not find rpc config " << func);
            return XCode::NotFoundRpcConfig;
        }
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode RpcNode::Call(const string &func, const Message &message, std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("not find rpc config " << func);
            return XCode::NotFoundRpcConfig;
        }
        requestData->mutable_data()->PackFrom(message);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }
}// namespace GameKeeper