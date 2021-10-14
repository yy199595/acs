#include "NetProxyComponent.h"

#include <Core/App.h>
#include <Scene/ProtocolComponent.h>
#include "NetSessionComponent.h"
#include<Service/ServiceNodeComponent.h>
#include<Service/ServiceNode.h>
#include <Scene/ActionComponent.h>
#include <Service/ServiceMgrComponent.h>
namespace Sentry
{

    bool NetProxyComponent::PushEventHandler(SocketEveHandler *eve)
    {
        if (eve == nullptr)
            return false;
        this->mNetEventQueue.Add(eve);
        return true;
    }

    bool NetProxyComponent::DestorySession(const std::string &address)
    {
		 auto handler = new MainSocketCloseHandler(address);
        return this->mNetWorkManager->PushEventHandler(handler);
    }

    bool NetProxyComponent::SendNetMessage(const std::string & address, com::DataPacket_Response & messageData)
    {
        auto methodId = (unsigned short)messageData.methodid();
        if(methodId <= 0)
        {
            return false;
        }
        TcpProxySession *tcpSession = this->GetProxySession(address);
        if (tcpSession == nullptr)
        {
            return false;
        }

        size_t size = 0;
        messageData.clear_methodid();
        MessageStream & sendStream = this->GetSendStream();
        sendStream << TYPE_RESPONSE << methodId << messageData;
        return tcpSession->SendMessageData(sendStream.Serialize(size), size);
    }

    MessageStream & NetProxyComponent::GetSendStream()
    {
        this->mSendStream.ResetPos();
        return this->mSendStream;
    }

    bool NetProxyComponent::SendNetMessage(const std::string & address, com::DataPacket_Request & messageData)
    {
        unsigned short methodId = (unsigned short)messageData.methodid();
        if(methodId <= 0)
        {
            return false;
        }
        TcpProxySession *tcpSession = this->GetProxySession(address);
        if (tcpSession == nullptr)
        {
            return false;
        }
        size_t size = 0;
        messageData.clear_methodid();
        MessageStream & sendStream = this->GetSendStream();
        sendStream << TYPE_REQUEST << methodId << messageData;
        return tcpSession->SendMessageData(sendStream.Serialize(size), size);
    }

	TcpProxySession * NetProxyComponent::Create(const std::string &address, const std::string &name)
    {		
		auto iter = this->mSessionMap.find(address);
		if (iter != this->mSessionMap.end())
		{
			return iter->second;
		}
		auto tcpSession = new TcpProxySession(name, address);

		this->mSessionMap.emplace(address, tcpSession);
		return tcpSession;
    }

    TcpProxySession *NetProxyComponent::GetProxySession(const std::string &address)
    {
        auto iter = this->mSessionMap.find(address);
        return iter != this->mSessionMap.end() ? iter->second : nullptr;
    }

    TcpProxySession *NetProxyComponent::DelProxySession(const std::string &address)
    {
        auto iter = this->mSessionMap.find(address);
        if (iter != this->mSessionMap.end())
        {
            TcpProxySession *session = iter->second;
            this->mSessionMap.erase(iter);
            return session;
        }
        return nullptr;
    }

    bool NetProxyComponent::Awake()
    {
		this->mReConnectTime = 3;
		this->mReConnectCount = 5;
		ServerConfig & config = App::Get().GetConfig();
		config.GetValue("NetWork", "ReConnectTime", this->mReConnectTime);
		config.GetValue("NetWork", "ReConnectCount", this->mReConnectCount);
        SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetComponent<NetSessionComponent>());
        SayNoAssertRetFalse_F(this->mActionComponent = this->GetComponent<ActionComponent>());

        SayNoAssertRetFalse_F(this->mServiceComponent = this->GetComponent<ServiceMgrComponent>());

        SayNoAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<ProtocolComponent>());

        std::vector<Component *> components;
        this->gameObject->GetComponents(components);
        for(Component * component : components)
        {
            if(auto reqHandler = dynamic_cast<IRequestMessageHandler*>(component))
            {
                this->mRequestMsgHandlers.emplace(component->GetTypeName(), reqHandler);
            }
            if(auto resHandler = dynamic_cast<IResponseMessageHandler*>(component))
            {
                this->mResponseMsgHandlers.emplace(component->GetTypeName(), resHandler);
            }
        }
        SayNoAssertRetFalse_F(!this->mRequestMsgHandlers.empty() && !this->mResponseMsgHandlers.empty());

        return true;
    }

	void NetProxyComponent::ConnectAfter(const std::string & address, bool isSuc)
	{
		auto iter = this->mSessionMap.find(address);
		if (iter != this->mSessionMap.end())
		{
			TcpProxySession *session = iter->second;
			if (session != nullptr)
			{
				session->SetActive(isSuc);
				this->OnConnectSuccessful(session);
				this->GetComponent<ServiceNodeComponent>()->GetServiceNode(address)->OnConnectNodeAfter();
			}
#ifdef _DEBUG
			SayNoDebugWarning("connect to " << address << isSuc ? " successful" : " failure");
#endif
		}
	}

	void NetProxyComponent::NewConnect(const std::string & address)
	{
		TcpProxySession *session = this->GetProxySession(address);
		if (session == nullptr)
		{
			session = new TcpProxySession(address);
			this->mSessionMap.emplace(address, session);
#ifdef SOEASY_DEBUG
			SayNoDebugInfo("new session connect [" << address << "]");
#endif
		}
		this->OnNewSessionConnect(session);
	}

	void NetProxyComponent::SessionError(const std::string & address)
	{
		auto iter = this->mSessionMap.find(address);
		if (iter != this->mSessionMap.end())
		{
			TcpProxySession *tcpSession = iter->second;
			if (tcpSession->IsNodeSession())
			{

				tcpSession->SetActive(false);
#ifdef SOEASY_DEBUG
				SayNoDebugError("[" << tcpSession->GetName() << " " << address << "] error");
#endif
				return;
			}
			delete tcpSession;
			this->mSessionMap.erase(iter);
#ifdef SOEASY_DEBUG
			SayNoDebugError(" remove session [" << address << "]");
#endif
		}
	}

	bool NetProxyComponent::ReceiveNewMessage(const std::string & address, SharedMessage message)
	{
        unsigned short methodId = 0;
        const char * msg = message->c_str();
        const size_t size = message->size();
        memcpy(&methodId, msg + 1, sizeof(methodId));
        const ProtocolConfig * protocolConfig = this->mProtocolComponent->GetProtocolConfig(methodId);

        if(protocolConfig == nullptr)
        {
            return false;
        }

        auto messageType = (DataMessageType)message->at(0);
        if(messageType ==  DataMessageType::TYPE_REQUEST)
        {
            const std::string &handler = protocolConfig->RequestHandler;
            auto iter = this->mRequestMsgHandlers.find(handler);
            if (iter != this->mRequestMsgHandlers.end())
            {
                return iter->second->OnRequestMessage(address, message);
            }
            return this->mServiceComponent->OnRequestMessage(address, message);
        }
        else if(messageType == DataMessageType::TYPE_RESPONSE)
        {
            const std::string & handler = protocolConfig->RequestHandler;
            auto iter = this->mResponseMsgHandlers.find(handler);
            if(iter != this->mResponseMsgHandlers.end())
            {
                return iter->second->OnResponseMessage(address, message);
            }
            this->mActionComponent->OnResponseMessage(address, message);
            return true;
        }
        return false;
	}

	void NetProxyComponent::OnSystemUpdate()
	{
		shared_ptr<TcpClientSession> pTcpSession = nullptr;
		SocketEveHandler *eveHandler = nullptr;
		this->mNetEventQueue.SwapQueueData();
		while (this->mNetEventQueue.PopItem(eveHandler))
		{
			eveHandler->RunHandler(this);
			delete eveHandler;
			eveHandler = nullptr;
		}
	}
}