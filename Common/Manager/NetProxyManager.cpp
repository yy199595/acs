#include "NetProxyManager.h"
#include "NetSessionManager.h"
#include "ServiceManager.h"
#include <Manager/ActionManager.h>
#include <Timer/TimerManager.h>

namespace Sentry
{

    bool NetProxyManager::AddNetSessionEvent(Net2MainEvent *eve)
    {
        if (eve == nullptr)
            return false;
        this->mNetEventQueue.AddItem(eve);
        return true;
    }

    bool NetProxyManager::DestorySession(const std::string &address)
    {
        Main2NetEvent *eve = new Main2NetEvent(SocketDectoryEvent, address);
        return this->mNetWorkManager->AddNetSessionEvent(eve);
    }

    bool NetProxyManager::SendMsgByAddress(const std::string &address, NetMessageProxy *msgData)
    {
        TcpProxySession *tcpSession = this->GetProxySession(address);
        if (tcpSession == nullptr || msgData == nullptr)
        {
            delete msgData;
            return false;
        }
        return tcpSession->SendMessageData(msgData);
    }

    bool NetProxyManager::ConnectByAddress(const std::string &address, const std::string &name)
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

    TcpProxySession *NetProxyManager::GetProxySession(const std::string &address)
    {
        auto iter = this->mSessionMap.find(address);
        return iter != this->mSessionMap.end() ? iter->second : nullptr;
    }

    TcpProxySession *NetProxyManager::DelProxySession(const std::string &address)
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

    bool NetProxyManager::OnInit()
    {
        SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());
        SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
        SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
        SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetSessionManager>());
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("NetWork", "ReConnectTime", this->mReConnectTime));
        this->mReConnectTime = this->mReConnectTime * 1000;
        return true;
    }

	void NetProxyManager::OnSystemUpdate()
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

    bool NetProxyManager::OnRecvMessage(const std::string &address, NetMessageProxy *messageData)
    {
        if (messageData->GetMessageType() < REQUEST_END)
        {
            return mServiceManager->HandlerMessage(address, messageData);
        }
        this->mActionManager->InvokeCallback(messageData);
        return true;
    }
}