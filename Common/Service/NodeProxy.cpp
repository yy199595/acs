#include "NodeProxy.h"
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
	NodeProxy::NodeProxy(int areaId, int nodeId, const std::string & name, const std::string & address, long long socketId)
		: mAreaId(areaId), mNodeId(nodeId), mAddress(address), mNodeName(name), mIsClose(false), mSocketId(socketId)
	{
        GKAssertRet_F(this->mRpcComponent = App::Get().GetComponent<RpcComponent>());
        GKAssertRet_F(this->mCorComponent = App::Get().GetComponent<CoroutineComponent>());
        GKAssertRet_F(this->mProtocolComponent = App::Get().GetComponent<RpcProtoComponent>());
        GKAssertRet_F(this->mResponseComponent = App::Get().GetComponent<RpcResponseComponent>());
        GKAssertRet_F(StringHelper::ParseIpAddress(address, this->mIp, this->mPort));

        if(this->mSocketId == 0)
        {
            this->mSocketId = this->mRpcComponent->NewSession(this->mNodeName, this->mIp, this->mPort);
        }
        this->mCorId = this->mCorComponent->StartCoroutine(&NodeProxy::LoopSendMessage, this);
	}

	bool NodeProxy::AddService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
			return true;
		}
		return false;
	}

    RpcConnector * NodeProxy::GetTcpSession()
    {
        auto localSession = this->mRpcComponent->GetLocalSession(this->mSocketId);

        if (!localSession->IsOpen())
        {
            for (int index = 0; index < 1; index++)
            {
                localSession->StartAsyncConnect();
                if (localSession->IsOpen())
                {
                    return localSession;
                }
            }
            return nullptr;
        }
        return localSession;
    }

    void NodeProxy::LoopSendMessage()
    {
        while (!this->mIsClose)
        {
            if (this->mWaitSendQueue.empty())
            {
                this->mCorComponent->YieldReturn();
            }
            RpcConnector *tcpLocalSession = this->GetTcpSession();
            if (tcpLocalSession == nullptr)
            {
                GKDebugError("node session [" << this->GetNodeName()
                                              << ":" << this->GetAddress() << "] connect error");
                this->mCorComponent->YieldReturn();
                continue;
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

    void NodeProxy::Destory()
    {
        this->mIsClose = true;
        this->mCorComponent->Resume(this->mCorId);
    }

    bool NodeProxy::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

    void NodeProxy::GetServicers(std::vector<std::string> & services)
    {
        services.clear();
        for(const std::string & name : this->mServiceArray)
        {
            services.emplace_back(name);
        }
    }

    XCode NodeProxy::Notice(const std::string &method)
    {
        auto config = this->mProtocolComponent->GetProtocolConfig(method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
		com::Rpc_Request * request = new com::Rpc_Request();
		request->set_methodid(config->MethodId);   
        this->AddRequestDataToQueue(request);
        return XCode::Successful;
    }

	XCode NodeProxy::Notice(const std::string &method, const Message &request)
	{
		com::Rpc_Request * data = this->CreateRequest(method);
        if(data == nullptr)
        {
            return XCode::NotFoundRpcConfig;
        }		
		data->mutable_data()->PackFrom(request);      
        this->AddRequestDataToQueue(data);
        return XCode::Successful;
	}

	com::Rpc_Request * NodeProxy::CreateRequest(const std::string & method)
	{
		auto config = this->mProtocolComponent->GetProtocolConfig(method);
		if (config == nullptr)
		{
			return nullptr;
		}
		com::Rpc_Request * request = new com::Rpc_Request();
		request->set_methodid(config->MethodId);
		return request;
	}

	XCode NodeProxy::Invoke(const std::string &method)
	{      
		com::Rpc_Request * request = this->CreateRequest(method);
		if (request == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(request->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler, handlerId))
        {
            return XCode::Failure;
        }
		request->set_rpcid(handlerId);     
        this->AddRequestDataToQueue(request);
        return cppCallHandler.StartCall();
	}

	XCode NodeProxy::Call(const std::string &method, Message &response)
	{
		com::Rpc_Request * request = this->CreateRequest(method);
		if (request == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(request->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }
		request->set_rpcid(handlerId);		
        this->AddRequestDataToQueue(request);
        return cppCallHandler.StartCall(response);
	}

	XCode NodeProxy::Invoke(const std::string &method, const Message &request)
	{
		com::Rpc_Request * requestData = this->CreateRequest(method);
		if (requestData == nullptr)
		{
			delete requestData;
			return XCode::NotFoundRpcConfig;
		}

        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(requestData->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }
		requestData->set_rpcid(handlerId);
		requestData->mutable_data()->PackFrom(request);
      
        this->AddRequestDataToQueue(requestData);
        return cppCallHandler.StartCall();
	}

	XCode NodeProxy::Call(const std::string &method, const Message &request, Message &response)
    {
		com::Rpc_Request * requestData = this->CreateRequest(method);
		if (requestData == nullptr)
		{
			delete requestData;
			return XCode::NotFoundRpcConfig;
		}
        unsigned int handlerId = 0;
		CppCallHandler cppCallHandler(requestData->methodid());
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }
		requestData->set_rpcid(handlerId);
		requestData->mutable_data()->PackFrom(request);
       
        this->AddRequestDataToQueue(requestData);
        return cppCallHandler.StartCall(response);
    }

    void NodeProxy::AddRequestDataToQueue(const com::Rpc_Request * requestData)
    {
        this->mWaitSendQueue.push(requestData);
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace GameKeeper