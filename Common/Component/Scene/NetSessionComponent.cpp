#include "NetSessionComponent.h"

#include <Core/App.h>
#include "ActionComponent.h"
#include "LuaScriptComponent.h"
#include "NetProxyComponent.h"
#include <Util/StringHelper.h>
#include <NetWork/TcpClientSession.h>
namespace Sentry
{
    NetSessionComponent::NetSessionComponent()
    {

    }


    bool NetSessionComponent::Awake()
    {
        SayNoAssertRetFalse_F(this->mNetProxyComponent = this->GetComponent<NetProxyComponent>());
        return true;
    }

    void NetSessionComponent::OnConnectComplete(TcpClientSession *session, bool isSuc)
    {
        const std::string &address = session->GetAddress();
		if (isSuc)
		{
			this->mRecvSessionQueue.push(address);
		}
		this->mNetProxyComponent->PushEventHandler(new NetSocketConnectHandler(address, isSuc));
    }

    void NetSessionComponent::OnSessionError(TcpClientSession *session)
    {
		const std::string &address = session->GetAddress();
        this->mNetProxyComponent->PushEventHandler(new NetErrorHandler(address));
    }


    bool NetSessionComponent::OnRecvMessage(TcpClientSession *session, const char *message, const size_t size)
    {
        if (message == nullptr || size == 0)
        {
            return false;
        }
		const std::string & address = session->GetAddress();
        PacketMapper *messageData = PacketMapper::Create(address, message, size);
        if (messageData == nullptr)
        {
            return false;
        }
		this->mRecvSessionQueue.push(address);
		return this->mNetProxyComponent->PushEventHandler(new NetReceiveNewMessageHandler(messageData));
    }

    void NetSessionComponent::OnSendMessageAfter(std::string *message)
    {
        if(message != nullptr)
        {
            message->clear();
            this->mSendBufferPool.Destory(message);
        }
    }

	bool NetSessionComponent::PushEventHandler(SocketEveHandler *eve)
	{
		if (eve == nullptr) return false;
		this->mNetEventQueue.Add(eve);
		return true;
	}

	TcpClientSession *NetSessionComponent::Create(shared_ptr<AsioTcpSocket> socket)
    {
		AsioContext & io = App::Get().GetTcpContext();
        TcpClientSession *session = new TcpClientSession(io, this, socket);

		const std::string & address = session->GetAddress();
        if (this->mSessionAdressMap.find(address) == this->mSessionAdressMap.end())
        {
            this->mRecvSessionQueue.push(address);
            this->mSessionAdressMap.emplace(address, session);
            this->mNetProxyComponent->PushEventHandler(new NetNewSocketConnectHandler(address));
            return session;
        }
        return nullptr;
    }

    TcpClientSession *NetSessionComponent::Create(const std::string &name, const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        if (iter != this->mSessionAdressMap.end())
        {
            return iter->second;
        }
        std::string ip;
        unsigned short port;
        if (StringHelper::ParseIpAddress(address, ip, port))
        {
			AsioContext & io = App::Get().GetTcpContext();
            TcpClientSession *session = new TcpClientSession(io, this, name, ip, port);
            this->mSessionAdressMap.emplace(address, session);
            return session;
        }
        return nullptr;
    }

    void NetSessionComponent::OnDestory()
    {
    }


    void NetSessionComponent::OnTcpContextUpdate(AsioContext &io)
    {
       
        while (!this->mRecvSessionQueue.empty())
        {
			const std::string & address = this->mRecvSessionQueue.front();
            TcpClientSession *tcpSession = this->GetSession(address);
            if (tcpSession != nullptr && tcpSession->IsActive())
            {
				tcpSession->StartReceiveMsg();
            }
            this->mRecvSessionQueue.pop();
        }

		this->mNetEventQueue.SwapQueueData();
		SocketEveHandler * eveHandler = nullptr;    
        while (this->mNetEventQueue.PopItem(eveHandler))
        {
			eveHandler->RunHandler(this);
			delete eveHandler;
        }
    }

    TcpClientSession *NetSessionComponent::GetSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        return iter != this->mSessionAdressMap.end() ? iter->second : nullptr;
    }

    bool NetSessionComponent::StartClose(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        if (iter != this->mSessionAdressMap.end())
        {
            TcpClientSession *session = iter->second;
            if (session != nullptr && session->IsActive())
            {
                session->StartClose();
            }
			delete session;
            this->mSessionAdressMap.erase(iter);
            SayNoDebugError("remove tcp session : " << address);
            return true;
        }
        return false;
    }
	bool NetSessionComponent::StartConnect(const std::string & address, const std::string & name)
	{
		TcpClientSession *session = this->GetSession(address);
		if (session == nullptr)
		{
			session = this->Create(name, address);
		}
		return session->StartConnect();
	}
	bool NetSessionComponent::StartSendMessage(PacketMapper * messageData)
	{
		const std::string & address = messageData->GetAddress();
		if (address.empty())
		{
			SayNoDebugFatal("address is null send failure");
			return false;
		}
		TcpClientSession *session = this->GetSession(address);
		if (session != nullptr)
		{
			size_t size = messageData->WriteToBuffer(this->mSendSharedBuffer, TCP_SEND_MAX_COUNT);
            if(size > 0)
            {
                std::string * message = this->mSendBufferPool.Create();
                message->append(this->mSendSharedBuffer, size);
                return session->SendPackage(message);
            }
		}
		return false;
	}
}// namespace Sentry