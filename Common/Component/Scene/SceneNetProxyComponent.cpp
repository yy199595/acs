#include "SceneNetProxyComponent.h"

#include <Core/App.h>
#include "SceneActionComponent.h"
#include "SceneSessionComponent.h"
#include <Timer/TimerComponent.h>
#include <Service/ServiceMgrComponent.h>
namespace Sentry
{

    bool SceneNetProxyComponent::AddNetSessionEvent(Net2MainEvent *eve)
    {
        if (eve == nullptr)
            return false;
        this->mNetEventQueue.Add(eve);
        return true;
    }

    bool SceneNetProxyComponent::DestorySession(const std::string &address)
    {
        Main2NetEvent *eve = new Main2NetEvent(SocketDectoryEvent, address);
        return this->mNetWorkManager->AddNetSessionEvent(eve);
    }

    bool SceneNetProxyComponent::SendMsgByAddress(const std::string &address, NetMessageProxy *msgData)
    {
        TcpProxySession *tcpSession = this->GetProxySession(address);
        if (tcpSession == nullptr || msgData == nullptr)
        {
            delete msgData;
            return false;
        }
        return tcpSession->SendMessageData(msgData);
    }

    bool SceneNetProxyComponent::ConnectByAddress(const std::string &address, const std::string &name)
    {
        auto iter = this->mConnectSessionMap.find(address);
        if (iter != this->mConnectSessionMap.end())
        {
            return true;
        }

        auto iter1 = this->mSessionMap.find(address);
        if (iter1 != this->mSessionMap.end())
        {
            return true;
        }

        TcpProxySession *tcpSession = new TcpProxySession(name, address);
        if (tcpSession != nullptr)
        {
            tcpSession->StartConnect();
            this->mConnectSessionMap.emplace(address, tcpSession);
            return true;
        }
        return false;
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
		ServerConfig & config = App::Get().GetConfig();
		this->mTimerManager = App::Get().GetTimerComponent();
		mServiceMgrComponent = Service::GetComponent<ServiceMgrComponent>();
        SayNoAssertRetFalse_F(this->mActionManager = Scene::GetComponent<SceneActionComponent>());
        SayNoAssertRetFalse_F(this->mNetWorkManager = Scene::GetComponent<SceneSessionComponent>());
        SayNoAssertRetFalse_F(config.GetValue("NetWork", "ReConnectTime", this->mReConnectTime));
        this->mReConnectTime = this->mReConnectTime * 1000;
        return true;
    }

	void SceneNetProxyComponent::OnSystemUpdate()
	{
		shared_ptr<TcpClientSession> pTcpSession = nullptr;
		Net2MainEvent *eve = nullptr;
		this->mNetEventQueue.SwapQueueData();
		while (this->mNetEventQueue.PopItem(eve))
		{
			const std::string &address = eve->GetAddress();
			if (eve->GetEventType() == Net2MainEventType::SocketConnectSuc)
			{
				auto iter = this->mConnectSessionMap.find(address);
				if (iter != this->mConnectSessionMap.end())
				{
					TcpProxySession *session = iter->second;
					this->mSessionMap.emplace(address, session);
					this->mConnectSessionMap.erase(iter);
#ifdef SOEASY_DEBUG
					SayNoDebugInfo("connect to " << address << " successful");
#endif
					this->OnConnectSuccessful(session);
				}
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketNewConnect)
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
			else if (eve->GetEventType() == Net2MainEventType::SocketConnectFail)
			{
				auto iter = this->mConnectSessionMap.find(address);
				if (iter != this->mConnectSessionMap.end())
				{
					TcpProxySession *session = iter->second;
					this->mTimerManager->AddTimer(this->mReConnectTime, &TcpProxySession::StartConnect, session);
#ifdef SOEASY_DEBUG
					const std::string &name = eve->GetName();
					SayNoDebugWarning("connect " << name << " " << address << "fail");
#endif
				}
				else
				{
					SayNoDebugError("not find address [" << address << "]");
				}
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketReceiveFail)
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
			else if (eve->GetEventType() == Net2MainEventType::SocketSendMsgFail)
			{
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketReceiveData)
			{
				NetMessageProxy *msgData = eve->GetMsgData();
				if (!this->OnRecvMessage(address, msgData))
				{
					TcpProxySession *session = this->DelProxySession(address);
					if (session != nullptr)
					{
						delete session;
						Main2NetEvent *e = new Main2NetEvent(SocketDectoryEvent, address);
						this->mNetWorkManager->AddNetSessionEvent(e);
					}
				}
			}
			delete eve;
		}
	}

    bool SceneNetProxyComponent::OnRecvMessage(const std::string &address, NetMessageProxy *messageData)
    {
        if (messageData->GetMessageType() < REQUEST_END)
        {			
			ServiceMgrComponent * serviceComponent = Service::GetComponent<ServiceMgrComponent>();
			return serviceComponent->HandlerMessage(address, messageData);    
        }
        this->mActionManager->InvokeCallback(messageData);
        return true;
    }
}