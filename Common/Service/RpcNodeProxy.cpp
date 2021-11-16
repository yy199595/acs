#include "RpcNodeProxy.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/RpcResponseComponent.h>
#include <Method/CallHandler.h>
#include <Scene/RpcProtoComponent.h>
#include <Util/StringHelper.h>
#include <Component/Scene/RpcComponent.h>
#include <google/protobuf/util/json_util.h>

namespace GameKeeper
{
	RpcNodeProxy::RpcNodeProxy(unsigned int id)
		:  mGlobalId(id), mIsClose(false), mSocketId(0)
	{
        GKAssertRet_F(this->mRpcComponent = App::Get().GetComponent<RpcComponent>());
        GKAssertRet_F(this->mCorComponent = App::Get().GetComponent<CoroutineComponent>());
        GKAssertRet_F(this->mProtocolComponent = App::Get().GetComponent<RpcProtoComponent>());
        GKAssertRet_F(this->mResponseComponent = App::Get().GetComponent<RpcResponseComponent>());
        this->mCorId = this->mCorComponent->StartCoroutine(&RpcNodeProxy::LoopSendMessage, this);
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
        this->mIsConnected = socketId == 0;
        this->mCorComponent->Resume(this->mCorId);
        return true;
    }

    RpcClient * RpcNodeProxy::GetTcpSession()
    {
        if(this->mSocketId == 0)
        {
           this->mSocketId = this->mRpcComponent->NewSession(this->mNodeName, this->mNodeIp, this->mNodePort);
           auto localRpcSession = this->mRpcComponent->GetLocalSession(this->mSocketId);
           if(localRpcSession != nullptr && localRpcSession->StartAsyncConnect())
           {
               GKDebugError("connect " << this->mNodeName << " successful");
               return localRpcSession;
           }
            return nullptr;
        }
        auto rpcClient = this->mRpcComponent->GetRpcSession(this->mSocketId);
        if(rpcClient != nullptr && rpcClient->IsOpen())
        {
            return rpcClient;
        }

        if(this->mIsConnected)
        {
            auto localRpcSession = this->mRpcComponent->GetLocalSession(this->mSocketId);
            if(localRpcSession != nullptr && localRpcSession->StartAsyncConnect())
            {
                return localRpcSession;
            }
        }
        return nullptr;
    }

    void RpcNodeProxy::LoopSendMessage()
    {
        while (!this->mIsClose)
        {
            if (this->mWaitSendQueue.empty())
            {
                this->mCorComponent->YieldReturn();
            }
            RpcClient *tcpLocalSession = this->GetTcpSession();
            while(tcpLocalSession == nullptr)
            {
                this->mCorComponent->Sleep(3000);
                tcpLocalSession = this->GetTcpSession();
            }
            for (int index = 0; index < 100 && !this->mWaitSendQueue.empty(); index++)
            {
                auto message = this->mWaitSendQueue.front();
                tcpLocalSession->StartSendProtocol(RPC_TYPE_REQUEST, message);
                this->mWaitSendQueue.pop();
            }
        }
        delete this;
    }

    void RpcNodeProxy::Destory()
    {
        this->mIsClose = true;
        this->mCorComponent->Resume(this->mCorId);
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
        auto config = this->mProtocolComponent->GetProtocolConfig(method);
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
		auto config = this->mProtocolComponent->GetProtocolConfig(method);
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

    void RpcNodeProxy::AddRequestDataToQueue(const com::Rpc_Request * requestData)
    {
        this->mWaitSendQueue.push(requestData);
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace GameKeeper