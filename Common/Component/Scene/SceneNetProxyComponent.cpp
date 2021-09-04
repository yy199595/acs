#include "SceneNetProxyComponent.h"

#include <Core/App.h>
#include "SceneActionComponent.h"
#include "SceneSessionComponent.h"
#include <Timer/TimerComponent.h>
#include<Service/ServiceNodeComponent.h>
#include <Service/ServiceMgrComponent.h>
#include<Service/ServiceNode.h>
namespace Sentry
{

    bool SceneNetProxyComponent::PushEventHandler(SocketEveHandler *eve)
    {
        if (eve == nullptr)
            return false;
        this->mNetEventQueue.Add(eve);
        return true;
    }

    bool SceneNetProxyComponent::DestorySession(const std::string &address)
    {
		MainSocketCloseHandler * handler = new MainSocketCloseHandler(address);
        return this->mNetWorkManager->PushEventHandler(handler);
    }

    bool SceneNetProxyComponent::SendNetMessage(PacketMapper *msgData)
    {
		const std::string &address = msgData->GetAddress();
		if (address.empty())
		{
			SayNoDebugFatal("address is null send failure");
			return false;
		}
        TcpProxySession *tcpSession = this->GetProxySession(address);
        if (tcpSession == nullptr)
        {
            delete msgData;
            return false;
        }
        return tcpSession->SendMessageData(msgData);
    }

	TcpProxySession * SceneNetProxyComponent::ConnectByAddress(const std::string &address, const std::string &name)
    {		
        auto iter = this->mConnectSessionMap.find(address);
        if (iter != this->mConnectSessionMap.end())
        {
			return iter->second;
        }
		else
		{
			TcpProxySession *tcpSession = new TcpProxySession(name, address);
			if (tcpSession != nullptr)
			{
				tcpSession->StartConnect();
				this->mConnectSessionMap.emplace(address, tcpSession);
				return tcpSession;
			}
		}
		return nullptr;
    }

    TcpProxySession *SceneNetProxyComponent::GetProxySession(const std::string &address)
    {
        auto iter = this->mSessionMap.find(address);
        return iter != this->mSessionMap.end() ? iter->second : nullptr;
    }

    TcpProxySession *SceneNetProxyComponent::DelProxySession(const std::string &address)
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

    bool SceneNetProxyComponent::Awake()
    {
		this->mReConnectTime = 3;
		this->mReConnectCount = 5;
		ServerConfig & config = App::Get().GetConfig();
		this->mTimerManager = App::Get().GetTimerComponent();
		config.GetValue("NetWork", "ReConnectTime", this->mReConnectTime);
		config.GetValue("NetWork", "ReConnectCount", this->mReConnectCount);
        SayNoAssertRetFalse_F(this->mActionManager = Scene::GetComponent<SceneActionComponent>());
        SayNoAssertRetFalse_F(this->mNetWorkManager = Scene::GetComponent<SceneSessionComponent>());     
        return true;
    }

	void SceneNetProxyComponent::ConnectSuccessful(const std::string & address)
	{
		auto iter = this->mConnectSessionMap.find(address);
		if (iter != this->mConnectSessionMap.end())
		{
			TcpProxySession *session = iter->second;
			if (session != nullptr)
			{
				session->SetActive(true);
				this->mConnectSessionMap.erase(iter);
				this->mSessionMap.emplace(address, session);

				this->OnConnectSuccessful(session);
				Service::GetComponent<ServiceNodeComponent>()->GetServiceNode(address)->OnConnectNodeAfter();
			}
#ifdef _DEBUG
			SayNoDebugWarning("connect to " << address << " successful");
#endif
		}
	}

	void SceneNetProxyComponent::ConnectFailure(const std::string & address)
	{
		auto iter = this->mConnectSessionMap.find(address);
		if (iter != this->mConnectSessionMap.end())
		{
			TcpProxySession *session = iter->second;
			if (session->GetConnectCount() < this->mReConnectCount)
			{
#ifdef _DEBUG
				const std::string &name = session->GetName();
				SayNoDebugError("connect " << name << " " << address << " failure "
					<< "reconnect in " << this->mReConnectTime << " atter [" << session->GetConnectCount() << "]");
#endif
				this->mTimerManager->AddTimer(this->mReConnectTime * 1000, &TcpProxySession::StartConnect, session);
			}
			else
			{
				session->SetActive(false);
				const std::string &name = session->GetName();
				SayNoDebugFatal("connect " << name << " " << address << " failure");
				Service::GetComponent<ServiceNodeComponent>()->GetServiceNode(address)->OnConnectNodeAfter();
			}
		}
		else
		{
			SayNoDebugError("not find address [" << address << "]");
		}
	}

	void SceneNetProxyComponent::NewConnect(const std::string & address)
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

	void SceneNetProxyComponent::SessionError(const std::string & address)
	{
		TcpProxySession *tcpSession = this->DelProxySession(address);
		if (tcpSession != nullptr)
		{
			if (tcpSession->IsNodeSession())
			{
				this->mConnectSessionMap.emplace(tcpSession->GetAddress(), tcpSession);
				this->mTimerManager->AddTimer(this->mReConnectTime,
					&TcpProxySession::StartConnect, tcpSession);
#ifdef SOEASY_DEBUG
				SayNoDebugError("receive message error re connect [" << tcpSession->GetConnectCount() << "]");
#endif
			}
			else
			{
				delete tcpSession;
			}
		}
	}

	void SceneNetProxyComponent::ReceiveNewMessage(PacketMapper * message)
	{
		if (!this->OnRecvMessage(message))
		{
			const std::string & address = message->GetAddress();
			TcpProxySession *session = this->DelProxySession(address);
			if (session != nullptr)
			{
				delete session;
				MainSocketCloseHandler * handler = new MainSocketCloseHandler(address);
				this->mNetWorkManager->PushEventHandler(handler);
			}
		}
	}

	void SceneNetProxyComponent::OnSystemUpdate()
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

    bool SceneNetProxyComponent::OnRecvMessage(PacketMapper *messageData)
    {
        if (messageData->GetMessageType() < REQUEST_END)
        {			
			ServiceMgrComponent * serviceComponent = Service::GetComponent<ServiceMgrComponent>();
			if (serviceComponent == nullptr)
			{
				return false;
			}
			return serviceComponent->HandlerMessage(messageData);    
        }
        this->mActionManager->InvokeCallback(messageData);
        return true;
    }
}