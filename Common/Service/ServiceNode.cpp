#include "ServiceNode.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/RpcResponseComponent.h>
#include <Method/CallHandler.h>
#include <Scene/ProtocolComponent.h>
#include <Util/StringHelper.h>
#include <Network/Rpc/RpcComponent.h>
#include <google/protobuf/util/json_util.h>
namespace GameKeeper
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string & name, const std::string & address, long long socketId)
		: mAreaId(areaId), mNodeId(nodeId), mAddress(address), mNodeName(name), mIsClose(false), mSocketId(socketId)
	{
        GKAssertRet_F(this->mRpcComponent = App::Get().GetComponent<RpcComponent>());
        GKAssertRet_F(this->mCorComponent = App::Get().GetComponent<CoroutineComponent>());
        GKAssertRet_F(this->mProtocolComponent = App::Get().GetComponent<ProtocolComponent>());
        GKAssertRet_F(this->mResponseComponent = App::Get().GetComponent<RpcResponseComponent>());
        GKAssertRet_F(StringHelper::ParseIpAddress(address, this->mIp, this->mPort));

        if(this->mSocketId == 0)
        {
            this->mSocketId = this->mRpcComponent->NewSession(this->mNodeName, this->mIp, this->mPort);
        }
        this->mCorId = this->mCorComponent->StartCoroutine(&ServiceNode::LoopSendMessage, this);
	}

	bool ServiceNode::AddService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
			return true;
		}
		return false;
	}

    RpcLocalSession * ServiceNode::GetTcpSession()
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

    void ServiceNode::LoopSendMessage()
    {
        while (!this->mIsClose)
        {
            if (this->mWaitSendQueue.empty())
            {
                this->mCorComponent->YieldReturn();
            }
            RpcLocalSession *tcpLocalSession = this->GetTcpSession();
            if (tcpLocalSession == nullptr)
            {
                GKDebugError("node session [" << this->GetNodeName()
                                              << ":" << this->GetAddress() << "] connect error");
                this->mCorComponent->YieldReturn();
                continue;
            }
            for (int index = 0; index < 100 && !this->mWaitSendQueue.empty(); index++)
            {
                std::string *message = this->mWaitSendQueue.front();
                tcpLocalSession->StartSendByString(message);
                this->mWaitSendQueue.pop();
            }
        }
        delete this;
    }

    void ServiceNode::Destory()
    {
        this->mIsClose = true;
        this->mCorComponent->Resume(this->mCorId);
    }

    bool ServiceNode::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

    void ServiceNode::GetServicers(std::vector<std::string> & services)
    {
        services.clear();
        for(const std::string & name : this->mServiceArray)
        {
            services.emplace_back(name);
        }
    }

    XCode ServiceNode::Notice(const std::string &service, const std::string &method)
    {
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
        this->mRequestData.Clear();
        this->mRequestData.set_methodid(config->MethodId);
        std::string * message = this->mRpcComponent->Serialize(this->mRequestData);
        if(message == nullptr)
        {
            return XCode::SerializationFailure;
        }
        this->PushMessage(message);
        return XCode::Successful;
    }

	XCode ServiceNode::Notice(const std::string &service, const std::string &method, const Message &request)
	{
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
        this->mRequestData.Clear();
        this->mRequestData.set_methodid(config->MethodId);
        if(!request.SerializeToString(&mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }
        std::string * message = this->mRpcComponent->Serialize(this->mRequestData);
        if(message == nullptr)
        {
            return XCode::SerializationFailure;
        }
        this->PushMessage(message);
        return XCode::Successful;
	}

	XCode ServiceNode::Invoke(const std::string &service, const std::string &method)
	{
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }

        unsigned int handlerId = 0;
        CppCallHandler cppCallHandler;
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }

        this->mRequestData.Clear();
        this->mRequestData.set_rpcid(handlerId);
        this->mRequestData.set_methodid(config->MethodId);
        std::string * message = this->mRpcComponent->Serialize(this->mRequestData);
        if(message == nullptr)
        {
            return XCode::SerializationFailure;
        }
        this->PushMessage(message);
        return cppCallHandler.StartCall();
	}

	XCode ServiceNode::Call(const std::string &service, const std::string &method, Message &response)
	{
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }

        unsigned int handlerId = 0;
        CppCallHandler cppCallHandler;
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }

        this->mRequestData.Clear();
        this->mRequestData.set_rpcid(handlerId);
        this->mRequestData.set_methodid(config->MethodId);
        std::string * message = this->mRpcComponent->Serialize(this->mRequestData);
        if(message == nullptr)
        {
            return XCode::SerializationFailure;
        }
        this->PushMessage(message);
        return cppCallHandler.StartCall(response);
	}

	XCode ServiceNode::Invoke(const std::string &service, const std::string &method, const Message &request)
	{
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }

        unsigned int handlerId = 0;
        CppCallHandler cppCallHandler;
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }

        if(!request.SerializeToString(&mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }

        this->mRequestData.Clear();
        this->mRequestData.set_rpcid(handlerId);
        this->mRequestData.set_methodid(config->MethodId);
        this->mRequestData.set_messagedata(mMessageBuffer);
        std::string * message = this->mRpcComponent->Serialize(this->mRequestData);
        if(message == nullptr)
        {
            return XCode::SerializationFailure;
        }
        this->PushMessage(message);
        return cppCallHandler.StartCall();
	}

	XCode ServiceNode::Call(const std::string &service, const std::string &method, const Message &request, Message &response)
    {
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if(config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }

        unsigned int handlerId = 0;
        CppCallHandler cppCallHandler;
        if(!this->mResponseComponent->AddCallHandler(&cppCallHandler,handlerId))
        {
            return XCode::Failure;
        }

        if(!request.SerializeToString(&mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }

        this->mRequestData.Clear();
        this->mRequestData.set_rpcid(handlerId);
        this->mRequestData.set_methodid(config->MethodId);
        this->mRequestData.set_messagedata(mMessageBuffer);
        std::string * message = this->mRpcComponent->Serialize(this->mRequestData);
        if(message == nullptr)
        {
            return XCode::SerializationFailure;
        }
        this->PushMessage(message);
        return cppCallHandler.StartCall(response);
    }

    void ServiceNode::PushMessage(std::string *msg)
    {
        this->mWaitSendQueue.push(msg);
        this->mCorComponent->Resume(this->mCorId);
    }
}// namespace GameKeeper