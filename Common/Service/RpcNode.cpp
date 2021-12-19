#include "RpcNode.h"
#include<Core/App.h>
#include<Coroutine/TaskComponent.h>
#include<Async/RpcTask/RpcTask.h>
#include<ServerRpc/RpcClientComponent.h>
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include<google/protobuf/util/json_util.h>
#include"ServerRpc/RpcComponent.h"
namespace GameKeeper
{
	RpcNode::RpcNode(int id)
		:  mGlobalId(id), mIsClose(false), mSocketId(0)
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
        this->mAreaId = nodeInfo.areaid();
        this->mNodeId = nodeInfo.nodeid();
        this->mNodeIp = nodeInfo.serverip();
        this->mNodeName = nodeInfo.servername();

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
            this->mSocketId = rpcClient->GetSocketProxy().GetSocketId();
            auto method = NewMethodProxy(&RpcNode::OnConnectAfter, this);
            rpcClient->StartConnect(this->mNodeIp, this->mNodePort, method);
        }
    }

    void RpcNode::OnConnectAfter()
    {
        while(!this->mWaitSendQueue.empty())
        {
            auto message  = this->mWaitSendQueue.front();
            if(!this->mRpcClientComponent->SendByAddress(this->mSocketId, message))
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

    XCode RpcNode::Notice(const std::string &method)
    {
        auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
        if (config == nullptr)
        {
            return XCode::NotFoundRpcConfig;
        }
        auto requestMessage = new com::Rpc_Request();
        requestMessage->set_methodid(config->MethodId);
        return XCode::Successful;
    }

	XCode RpcNode::Notice(const std::string &method, const Message &request)
	{
        auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
        if (config == nullptr)
        {
            return XCode::NotFoundRpcConfig;
        }
        auto requestMessage = new com::Rpc_Request();
        requestMessage->set_methodid(config->MethodId);
        requestMessage->mutable_data()->PackFrom(request);
        return XCode::Successful;
	}

	com::Rpc_Request * RpcNode::NewRequest(const std::string & method, int & methodId)
	{
		auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
		if (config == nullptr)
		{
			return nullptr;
		}
        methodId = config->MethodId;
        auto request = new com::Rpc_Request();
		request->set_methodid(config->MethodId);
        if(!this->mRpcClientComponent->SendByAddress(this->mSocketId, request))
        {
            this->ConnectToNode();
            this->mWaitSendQueue.push(request);
        }
        return request;
	}

    std::shared_ptr<RpcTask> RpcNode::NewRpcTask(const std::string &method)
    {
        int methodId = 0;
        auto requestData = this->NewRequest(method, methodId);
        if (requestData == nullptr)
        {
            LOG_ERROR("not find rpc config " << method);
            return std::make_shared<RpcTask>(XCode::NotFoundRpcConfig);
        }
        std::shared_ptr<RpcTask> rpcTask(new RpcTask(methodId));
        requestData->set_rpcid(rpcTask->GetTaskId());
        return rpcTask;
    }

    std::shared_ptr<RpcTask> RpcNode::NewRpcTask(const std::string &method, const Message &message)
    {
        int methodId = 0;
        auto requestData = this->NewRequest(method, methodId);
        if (requestData == nullptr)
        {
            LOG_ERROR("not find rpc config " << method);
            return std::make_shared<RpcTask>(XCode::NotFoundRpcConfig);
        }
        std::shared_ptr<RpcTask> rpcTask(new RpcTask(methodId));

        requestData->mutable_data()->PackFrom(message);
        requestData->set_rpcid(rpcTask->GetTaskId());
        return rpcTask;
    }

    XCode RpcNode::Invoke(const string &func)
    {
        return this->NewRpcTask(func)->GetCode();
    }

    XCode RpcNode::Invoke(const string &func, const Message &request)
    {
        return this->NewRpcTask(func, request)->GetCode();
    }
}// namespace GameKeeper