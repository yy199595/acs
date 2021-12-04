#include "RpcNodeProxy.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Async/RpcTask/ProtoRpcTask.h>
#include <ProtoRpc/ProtoRpcClientComponent.h>
#include <Util/StringHelper.h>
#include <Scene/RpcConfigComponent.h>
#include <google/protobuf/util/json_util.h>
#include"ProtoRpc//ProtoRpcComponent.h"
namespace GameKeeper
{
	RpcNodeProxy::RpcNodeProxy(int id)
		:  mGlobalId(id), mIsClose(false), mSocketId(0)
	{
        this->mRpcComponent = App::Get().GetComponent<ProtoRpcComponent>();
        this->mRpcClientComponent = App::Get().GetComponent<ProtoRpcClientComponent>();
        LOG_CHECK_RET(this->mCorComponent = App::Get().GetComponent<CoroutineComponent>());
        LOG_CHECK_RET(this->mRpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>());
	}

	bool RpcNodeProxy::AddService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
			return true;
		}
		return false;
	}

    bool RpcNodeProxy::UpdateNodeProxy(const s2s::NodeInfo &nodeInfo,long long socketId)
    {
        auto iter = nodeInfo.listeners().find("rpc");
        if(iter == nodeInfo.listeners().end())
        {
            return false;
        }

        this->mSocketId = socketId;
        this->mServiceArray.clear();
        this->mAreaId = nodeInfo.areaid();
        this->mNodeId = nodeInfo.nodeid();
        this->mNodeIp = nodeInfo.serverip();
        this->mNodeName = nodeInfo.servername();

        this->Init(nodeInfo.servername());
        this->mNodePort = (unsigned short)iter->second;
        for(const std::string & name : nodeInfo.services())
        {
            this->mServiceArray.insert(name);
        }
        this->mNodeInfo.CopyFrom(nodeInfo);
        if(this->mSocketId != 0)
        {
            this->OnNodeSessionRefresh();
        }
        return true;
    }

    ProtoRpcClient * RpcNodeProxy::GetTcpSession()
    {
        if(this->mSocketId == 0)
        {
           auto rpcClient = this->mRpcClientComponent->NewSession(this->mNodeName);
           if(rpcClient != nullptr)
           {
               this->mSocketId = rpcClient->GetSocketProxy().GetSocketId();
			   auto method = NewMethodProxy(&RpcNodeProxy::OnNodeSessionRefresh, this);
               rpcClient->StartConnect(this->mNodeIp, this->mNodePort, method);
           }
            return rpcClient;
        }
        return this->mRpcClientComponent->GetRpcSession(this->mSocketId);
    }

    void RpcNodeProxy::OnNodeSessionRefresh()
    {
        ProtoRpcClient *tcpLocalSession = this->GetTcpSession();
        if(!tcpLocalSession->IsOpen())
        {
            LOG_ERROR(this->mNodeName << " socket error");
            return;
        }
        while(!this->mWaitSendQueue.empty())
        {
            const Message * message = this->mWaitSendQueue.front();
            tcpLocalSession->StartSendProtocol(TYPE_REQUEST, message);
            this->mWaitSendQueue.pop();
        }
    }

    void RpcNodeProxy::Destory()
    {
        size_t size = this->mWaitSendQueue.size();
        this->mRpcClientComponent->CloseSession(this->mSocketId);
        if(size > 0)
        {
            LOG_ERROR("send queue has " << size << " data");
        }
        delete this;
    }

    bool RpcNodeProxy::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

    void RpcNodeProxy::GetServices(std::vector<std::string> & services)
    {
        services.clear();
        for(const std::string & name : this->mServiceArray)
        {
            services.emplace_back(name);
        }
    }

    XCode RpcNodeProxy::Notice(const std::string &method)
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

	XCode RpcNodeProxy::Notice(const std::string &method, const Message &request)
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

	com::Rpc_Request * RpcNodeProxy::CreateProtoRequest(const std::string & method, int & methodId)
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

    bool RpcNodeProxy::SendRequestData(const com::Rpc_Request * message)
    {
        ProtoRpcClient *rpcClient = this->GetTcpSession();
        if (rpcClient != nullptr && rpcClient->IsOpen())
        {
            rpcClient->StartSendProtocol(TYPE_REQUEST, message);
            return true;
        } else if (rpcClient->GetSocketType() == SocketType::LocalSocket)
        {
            if (!rpcClient->IsConnected())
            {
                auto method = NewMethodProxy(&RpcNodeProxy::OnNodeSessionRefresh, this);
                rpcClient->StartConnect(this->mNodeIp, this->mNodePort, method);
            }
        }
        this->mWaitSendQueue.push(message);
        return false;
    }

    std::shared_ptr<CppProtoRpcTask> RpcNodeProxy::SpawnProtoTask(const std::string &method)
    {
        int methodId = 0;
        auto requestData = this->CreateProtoRequest(method, methodId);
        if (requestData == nullptr)
        {
            return std::make_shared<CppProtoRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->SendRequestData(requestData);
        return std::make_shared<CppProtoRpcTask>(methodId, requestData->rpcid());
    }

    std::shared_ptr<CppProtoRpcTask> RpcNodeProxy::SpawnProtoTask(const std::string &method, const Message &message)
    {
        int methodId = 0;
        auto requestData = this->CreateProtoRequest(method, methodId);
        if (requestData == nullptr)
        {
            return std::make_shared<CppProtoRpcTask>(XCode::NotFoundRpcConfig);
        }
        requestData->mutable_data()->PackFrom(message);
        this->SendRequestData(requestData);
        return std::make_shared<CppProtoRpcTask>(methodId, requestData->rpcid());
    }
}// namespace GameKeeper