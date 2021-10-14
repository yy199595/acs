#include "ServiceNode.h"
#include <Core/App.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/ActionComponent.h>
#include <Scene/NetProxyComponent.h>
#include <NetWork/NetWorkRetAction.h>
#include <Scene/ProtocolComponent.h>
#include <Util/StringHelper.h>
#include <google/protobuf/util/json_util.h>
namespace Sentry
{
	ServiceNode::ServiceNode(int areaId, int nodeId, const std::string name, const std::string address)
		: mAreaId(areaId), mNodeId(nodeId), mAddress(address), mNodeName(name), mIsClose(false)
	{
		NetProxyComponent * component = App::Get().GetComponent<NetProxyComponent>();
        SayNoAssertRet_F(this->mCorComponent = App::Get().GetComponent<CoroutineComponent>());
		SayNoAssertRet_F(this->mActionManager = App::Get().GetComponent<ActionComponent>());
        SayNoAssertRet_F(this->mNetProxyComponent = App::Get().GetComponent<NetProxyComponent>());
        SayNoAssertRet_F(this->mProtocolComponent = App::Get().GetComponent<ProtocolComponent>());
        SayNoAssertRet_F(StringHelper::ParseIpAddress(address, this->mIp, this->mPort));
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

    TcpProxySession * ServiceNode::GetTcpSession()
    {
        TcpProxySession *tcpProxySession = this->mNetProxyComponent->Create(this->mAddress, this->mNodeName);
        if (!tcpProxySession->IsActive())
        {
            unsigned int corId = this->mCorComponent->GetCurrentCorId();
            if (corId == 0)
            {
                return nullptr;
            }
            this->mCoroutines.push(corId);
            tcpProxySession->StartConnect();
            this->mCorComponent->YieldReturn();
        }
        return this->mNetProxyComponent->GetProxySession(this->mAddress);
    }

    bool ServiceNode::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

	void ServiceNode::OnConnectNodeAfter()
	{
		while(!this->mCoroutines.empty())
        {
            unsigned int corId = this->mCoroutines.front();
            this->mCoroutines.pop();
            this->mCorComponent->Resume(corId);
        }
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
        return XCode::Successful;
    }

	XCode ServiceNode::Notice(const std::string &service, const std::string &method, const Message &request)
	{
        return XCode::Successful;
	}

	XCode ServiceNode::Invoke(const std::string &service, const std::string &method)
	{
        return XCode::Successful;
	}

	XCode ServiceNode::Call(const std::string &service, const std::string &method, Message &response)
	{
        return XCode::Successful;
	}

	XCode ServiceNode::Invoke(const std::string &service, const std::string &method, const Message &request)
	{
        return XCode::Successful;
	}

	XCode ServiceNode::Call(const std::string &service, const std::string &method, const Message &request, Message &response)
    {
        auto config = this->mProtocolComponent->GetProtocolConfig(service, method);
        if (config == nullptr)
        {
            return XCode::CallFunctionNotExist;
        }
        TcpProxySession *tcpProxySession = this->GetTcpSession();
        if (!tcpProxySession->IsActive())
        {
            return XCode::SendMessageFail;
        }
        if (!request.SerializeToString(&mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }
        size_t size = 0;
        com::DataPacket_Request requestData;
        requestData.set_messagedata(mMessageBuffer);
        auto rpcCallback = NetWorkWaitCorAction::Create();
        requestData.set_rpcid(this->mActionManager->AddCallback(rpcCallback));

        MessageStream &messageStream = this->mNetProxyComponent->GetSendStream();
        messageStream << DataMessageType::TYPE_REQUEST << config->MethodId << requestData;

        if(!tcpProxySession->SendMessageData(messageStream.Serialize(size), size))
        {
            return XCode::SendMessageFail;
        }
        this->mCorComponent->YieldReturn();

        if(rpcCallback->GetCode() == XCode::Successful)
        {
            response.ParseFromString(rpcCallback->GetMsgData());
            return XCode::Successful;
        }
        return rpcCallback->GetCode();
    }

	XCode ServiceNode::SendRpcMessage(SharedMessage message)
	{
		auto rpcCallback = NetWorkWaitCorAction::Create();
//		if (!message->SetRpcId(this->mActionManager->AddCallback(rpcCallback)))
//		{
//			return XCode::Failure;
//		}
		//this->AddMessageToQueue(message);
		return rpcCallback->GetCode();
	}

	XCode ServiceNode::SendRpcMessage(SharedMessage message, Message & response)
	{
		auto rpcCallback = NetWorkWaitCorAction::Create();
//		if (!message->SetRpcId(this->mActionManager->AddCallback(rpcCallback)))
//		{
//			return XCode::Failure;
//		}
		//this->AddMessageToQueue(message);
		if (rpcCallback->GetCode() == XCode::Successful)
		{
			const std::string & data = rpcCallback->GetMsgData();
			if (!response.ParseFromString(data))
			{
				SayNoDebugFatal("parse response message error type : " << response.GetTypeName());
				return XCode::ParseMessageError;
			}
		}
		return rpcCallback->GetCode();
	}
}// namespace Sentry