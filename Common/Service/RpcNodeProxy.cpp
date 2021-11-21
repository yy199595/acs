#include "RpcNodeProxy.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/RpcResponseComponent.h>
#include <Method/CallHandler.h>
#include <Scene/ProtoRpcComponent.h>
#include <Util/StringHelper.h>
#include <Scene/RpcConfigComponent.h>
#include <google/protobuf/util/json_util.h>

namespace GameKeeper
{
	RpcNodeProxy::RpcNodeProxy(int id)
		:  mGlobalId(id), mIsClose(false), mSocketId(0)
	{
        GKAssertRet_F(this->mRpcComponent = App::Get().GetComponent<ProtoRpcComponent>());
        GKAssertRet_F(this->mCorComponent = App::Get().GetComponent<CoroutineComponent>());
        GKAssertRet_F(this->mRpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>());
        GKAssertRet_F(this->mResponseComponent = App::Get().GetComponent<RpcResponseComponent>());
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
           auto rpcClient = this->mRpcComponent->NewSession(this->mNodeName);
           if(rpcClient != nullptr)
           {
               this->mSocketId = rpcClient->GetSocketProxy().GetSocketId();
			   auto method = NewMethodProxy(&RpcNodeProxy::OnNodeSessionRefresh, this);
               rpcClient->StartConnect(this->mNodeIp, this->mNodePort, method);
           }
            return rpcClient;
        }
        return this->mRpcComponent->GetRpcSession(this->mSocketId);
    }

    void RpcNodeProxy::OnNodeSessionRefresh()
    {
        ProtoRpcClient *tcpLocalSession = this->GetTcpSession();
        if(!tcpLocalSession->IsOpen())
        {
            GKDebugError(this->mNodeName << " socket error");
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
        this->mRpcComponent->CloseSession(this->mSocketId);
        if(size > 0)
        {
            GKDebugError("send queue has " << size << " data");
        }
        delete this;
    }

    bool RpcNodeProxy::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

    void RpcNodeProxy::GetServicers(std::vector<std::string> & services)
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
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
		auto request = new com::Rpc_Request();
		request->set_methodid(config->MethodId);   
        this->AddRequestDataToQueue(request);
        return XCode::Successful;
    }

	XCode RpcNodeProxy::Notice(const std::string &method, const Message &request)
	{
		auto data = this->CreateRequest(method);
        if(data == nullptr)
        {
            return XCode::NotFoundRpcConfig;
        }		
		data->mutable_data()->PackFrom(request);      
        this->AddRequestDataToQueue(data);
        return XCode::Successful;
	}

	com::Rpc_Request * RpcNodeProxy::CreateRequest(const std::string & method)
	{
		auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
		if (config == nullptr)
		{
			return nullptr;
		}
        auto request = new com::Rpc_Request();
		request->set_methodid(config->MethodId);
		return request;
	}

	XCode RpcNodeProxy::Invoke(const std::string &method)
	{      
		auto request = this->CreateRequest(method);
		if (request == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(request->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler, handlerId))
        {
            return XCode::AddRpcCallbackFailure;
        }
		request->set_rpcid(handlerId);     
        this->AddRequestDataToQueue(request);
        return cppCallHandler.StartCall();
	}

	XCode RpcNodeProxy::Call(const std::string &method, Message &response)
	{
		auto request = this->CreateRequest(method);
		if (request == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(request->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::AddRpcCallbackFailure;
        }
		request->set_rpcid(handlerId);		
        this->AddRequestDataToQueue(request);
        return cppCallHandler.StartCall(response);
	}

	XCode RpcNodeProxy::Invoke(const std::string &method, const Message &request)
	{
        auto requestData = this->CreateRequest(method);
		if (requestData == nullptr)
		{
			delete requestData;
			return XCode::NotFoundRpcConfig;
		}

        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(requestData->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::AddRpcCallbackFailure;
        }
		requestData->set_rpcid(handlerId);
		requestData->mutable_data()->PackFrom(request);
      
        this->AddRequestDataToQueue(requestData);
        return cppCallHandler.StartCall();
	}

	XCode RpcNodeProxy::Call(const std::string &method, const Message &request, Message &response)
    {
        auto requestData = this->CreateRequest(method);
		if (requestData == nullptr)
		{
			delete requestData;
			return XCode::NotFoundRpcConfig;
		}
        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(requestData->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::AddRpcCallbackFailure;
        }

		requestData->set_rpcid(handlerId);
		requestData->mutable_data()->PackFrom(request);

        this->AddRequestDataToQueue(requestData);
        return cppCallHandler.StartCall(response);
    }

    bool RpcNodeProxy::AddRequestDataToQueue(const com::Rpc_Request * requestData)
    {
        ProtoRpcClient * rpcClient = this->GetTcpSession();
        if(rpcClient != nullptr || rpcClient->IsOpen())
        {
            rpcClient->StartSendProtocol(TYPE_REQUEST, requestData);
            return true;
        }
        if(rpcClient->GetSocketType() == SocketType::LocalSocket)
        {
            if(!rpcClient->IsConnected())
            {
				auto method = NewMethodProxy(&RpcNodeProxy::OnNodeSessionRefresh, this);
                rpcClient->StartConnect(this->mNodeIp, this->mNodePort, method);
            }
        }
        this->mWaitSendQueue.push(requestData);
        return false;
    }
}// namespace GameKeeper