#include "RpcNode.h"
#include<Core/App.h>
#include<Coroutine/TaskComponent.h>
#include<Async/RpcTask/ProtoRpcTask.h>
#include<ServerRpc/ProtoRpcClientComponent.h>
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include<google/protobuf/util/json_util.h>
#include"ServerRpc/ProtoRpcComponent.h"
namespace GameKeeper
{
	RpcNode::RpcNode(int id)
		:  mGlobalId(id), mIsClose(false), mSocketId(0)
	{
        this->mRpcComponent = App::Get().GetComponent<ProtoRpcComponent>();
        this->mRpcClientComponent = App::Get().GetComponent<ProtoRpcClientComponent>();
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

    ProtoRpcClient * RpcNode::GetTcpSession()
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
        return rpcClient;
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

        this->SendRequestData(requestMessage);
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
        this->SendRequestData(requestMessage);
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
        request->set_rpcid(this->mRpcComponent->GetRpcTaskId());
		return request;
	}

    bool RpcNode::SendRequestData(com::Rpc_Request * message)
    {
        auto rpcClient = this->GetTcpSession();
        if(!rpcClient->SendToServer(message))
        {
            this->mWaitSendQueue.push(message);
            return false;
        }
        return true;
    }

    std::shared_ptr<CppProtoRpcTask> RpcNode::NewRpcTask(const std::string &method)
    {
        int methodId = 0;
        auto requestData = this->NewRequest(method, methodId);
        if (requestData == nullptr)
        {
            LOG_ERROR("not find rpc config " << method);
            return std::make_shared<CppProtoRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->SendRequestData(requestData);
        return std::make_shared<CppProtoRpcTask>(methodId, requestData->rpcid());
    }

    std::shared_ptr<CppProtoRpcTask> RpcNode::NewRpcTask(const std::string &method, const Message &message)
    {
        int methodId = 0;
        auto requestData = this->NewRequest(method, methodId);
        if (requestData == nullptr)
        {
            LOG_ERROR("not find rpc config " << method);
            return std::make_shared<CppProtoRpcTask>(XCode::NotFoundRpcConfig);
        }
        requestData->mutable_data()->PackFrom(message);
        this->SendRequestData(requestData);
        return std::make_shared<CppProtoRpcTask>(methodId, requestData->rpcid());
    }

    XCode RpcNode::Call(const string &func)
    {
        return this->NewRpcTask(func)->Await();
    }

    XCode RpcNode::Call(const string &func, const Message &request)
    {
        return this->NewRpcTask(func, request)->Await();
    }

    XCode RpcNode::Call(const string &func, std::shared_ptr<Message> response)
    {
        auto rpcTask = this->NewRpcTask(func);
        return rpcTask->Await(std::move(response));
    }

    XCode RpcNode::Call(const string &func, const Message &request, std::shared_ptr<Message> response)
    {
        auto rpcTask = this->NewRpcTask(func, request);
        return rpcTask->Await(std::move(response));
    }
}// namespace GameKeeper