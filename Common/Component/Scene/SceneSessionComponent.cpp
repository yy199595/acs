#include "SceneSessionComponent.h"

#include <Core/App.h>
#include "SceneActionComponent.h"
#include "SceneScriptComponent.h"
#include "SceneListenComponent.h"
#include "SceneNetProxyComponent.h"
#include <Util/StringHelper.h>
#include <NetWork/TcpClientSession.h>
namespace Sentry
{
    SceneSessionComponent::SceneSessionComponent()
    {

    }

    bool SceneSessionComponent::Awake()
    {
        SayNoAssertRetFalse_F(this->mNetProxyManager = Scene::GetComponent<SceneNetProxyComponent>());
        SayNoAssertRetFalse_F(this->mLocalActionManager = Scene::GetComponent<SceneActionComponent>());
        return true;
    }

    void SceneSessionComponent::OnConnectSuccess(TcpClientSession *session)
    {
        const std::string &address = session->GetAddress();
        const std::string &name = session->GetSessionName();
        Net2MainEvent *eve = new Net2MainEvent(SocketConnectSuc, address, name);
        if (eve != nullptr)
        {
            this->mRecvSessionQueue.push(address);
            this->mNetProxyManager->AddNetSessionEvent(eve);
        }
    }

    void SceneSessionComponent::OnSessionError(TcpClientSession *session, Net2MainEventType type)
    {

        const std::string &address = session->GetAddress();
        const std::string &name = session->GetSessionName();
        Net2MainEvent *eve = new Net2MainEvent(type, address, name);
        this->mNetProxyManager->AddNetSessionEvent(eve);
    }


    bool SceneSessionComponent::OnRecvMessage(TcpClientSession *session, const char *message, const size_t size)
    {
        if (message == nullptr || size == 0)
        {
            return false;
        }
		const std::string & address = session->GetAddress();
        NetMessageProxy *messageData = NetMessageProxy::Create(address, message, size);
        if (messageData == nullptr)
        {
            return false;
        }

        const std::string &name = session->GetSessionName();
        this->mRecvSessionQueue.push(session->GetAddress());
        Net2MainEvent *eve = new Net2MainEvent(SocketReceiveData, session->GetAddress(), name, messageData);
        return this->mNetProxyManager->AddNetSessionEvent(eve);
    }

    bool SceneSessionComponent::OnSendMessageError(TcpClientSession *session, const char *message, const size_t size)
    {

        if (message == nullptr || size == 0)
        {
            return false;
        }
		const std::string & address = session->GetAddress();
        NetMessageProxy *messageData = NetMessageProxy::Create(address, message, size);
        if (messageData == nullptr)
        {
            return false;
        }

        const std::string &name = session->GetSessionName();
        Net2MainEvent *eve = new Net2MainEvent(SocketSendMsgFail, address, name, messageData);
        return this->mNetProxyManager->AddNetSessionEvent(eve);
    }

    bool SceneSessionComponent::AddNetSessionEvent(Main2NetEvent *eve)
    {

        if (eve->GetEventType() == SocketSendMsgEvent)
        {
            if (eve->GetMsgData() == nullptr)
            {
                return false;
            }
        }
        this->mNetEventQueue.Add(eve);
        return true;
    }

    TcpClientSession *SceneSessionComponent::Create(shared_ptr<AsioTcpSocket> socket)
    {
		AsioContext & io = App::Get().GetNetContext();
        TcpClientSession *session = new TcpClientSession(io, this, socket);
        if (this->mSessionAdressMap.find(session->GetAddress()) == this->mSessionAdressMap.end())
        {
            this->mRecvSessionQueue.push(session->GetAddress());
            this->mSessionAdressMap.emplace(session->GetAddress(), session);
            Net2MainEvent *eve = new Net2MainEvent(SocketNewConnect, session->GetAddress());
            this->mNetProxyManager->AddNetSessionEvent(eve);
            return session;
        }
        return nullptr;
    }

    TcpClientSession *SceneSessionComponent::Create(const std::string &name, const std::string &address)
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
			AsioContext & io = App::Get().GetNetContext();
            TcpClientSession *session = new TcpClientSession(io, this, name, ip, port);
            this->mSessionAdressMap.emplace(address, session);
            return session;
        }
        return nullptr;
    }

    void SceneSessionComponent::OnDestory()
    {
    }


    void SceneSessionComponent::OnNetSystemUpdate(AsioContext &io)
    {
        Main2NetEvent *sessionEvent = nullptr;
        while (!this->mRecvSessionQueue.empty())
        {
            TcpClientSession *tcpSession = this->GetSession(this->mRecvSessionQueue.front());
            if (tcpSession != nullptr && tcpSession->IsActive())
            {
                tcpSession->StartReceiveMsg();
            }
            this->mRecvSessionQueue.pop();
        }

        this->mNetEventQueue.SwapQueueData();//处理主线程过来的数据
        while (this->mNetEventQueue.PopItem(sessionEvent))
        {
            this->HandlerMainThreadEvent(sessionEvent);
            delete sessionEvent;
        }
    }


	void SceneSessionComponent::HandlerMainThreadEvent(Main2NetEvent *eve)//处理主线程过来的事件
	{
		if (eve == nullptr)
			return;
		const std::string &address = eve->GetAddress();
		if (eve->GetEventType() == SocketDectoryEvent)
		{
			this->DescorySession(address);
		}
		else if (eve->GetEventType() == SocketConnectEvent)
		{
			const std::string &name = eve->GetName();
			TcpClientSession *session = this->Create(name, address);
			if (session == nullptr || !session->StartConnect())
			{
				Net2MainEvent *eve = new Net2MainEvent(SocketConnectFail, address, name);
				this->mNetProxyManager->AddNetSessionEvent(eve);
			}
		}
		else if (eve->GetEventType() == SocketSendMsgEvent)
		{
			TcpClientSession *session = this->GetSession(address);
			if (session != nullptr)
			{
				NetMessageProxy *messageData = eve->GetMsgData();
				size_t size = messageData->WriteToBuffer(this->mSendSharedBuffer, ASIO_TCP_SEND_MAX_COUNT);
				if (size == 0)
				{
					const std::string &name = session->GetSessionName();
					Net2MainEvent *eve = new Net2MainEvent(Net2MainEventType::SocketSendMsgFail, address, name,
						messageData);
					this->mNetProxyManager->AddNetSessionEvent(eve);
					return;
				}
				session->SendPackage(std::make_shared<std::string>(this->mSendSharedBuffer, size));
			}
		}
	}

    TcpClientSession *SceneSessionComponent::GetSession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        return iter != this->mSessionAdressMap.end() ? iter->second : nullptr;
    }

    bool SceneSessionComponent::DescorySession(const std::string &address)
    {
        auto iter = this->mSessionAdressMap.find(address);
        if (iter != this->mSessionAdressMap.end())
        {
            TcpClientSession *session = iter->second;
            if (session != nullptr && session->IsActive())
            {
                session->StartClose();
            }
            this->mSessionAdressMap.erase(iter);
            SayNoDebugError("remove tcp session : " << address);
            return true;
        }
        return false;
    }
}// namespace Sentry