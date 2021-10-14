#include "TcpProxySession.h"
#include <Core/App.h>
#include <Scene/NetProxyComponent.h>
#include <Scene/NetSessionComponent.h>
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{

    TcpProxySession::TcpProxySession(const std::string &address)
    {
		this->mIsActive = true;
        this->mConnectCount = 0;
        this->mAddress = address;
        this->mSessionType = SessionClient;    
        SayNoAssertRet_F(this->mNetManager = App::Get().GetComponent<NetSessionComponent>());
    }

    TcpProxySession::TcpProxySession(const std::string &name, const std::string &address)
    {
        this->mName = name;
        this->mConnectCount = 0;
		this->mIsActive = false;
        this->mAddress = address;
        this->mSessionType = SessionNode;
		SayNoAssertRet_F(this->mNetManager = App::Get().GetComponent<NetSessionComponent>());
    }

    TcpProxySession::~TcpProxySession()
    {    
		auto handler = new MainSocketCloseHandler(mAddress);
        this->mNetManager->PushEventHandler(handler);
#ifdef SOEASY_DEBUG
        SayNoDebugError("desctory session [" << this->mAddress << "]");
#endif
    }

    bool TcpProxySession::SendMessageData(SharedMessage message)
    {
        if (message == nullptr || message->size() <= 3)
        {
            return false;
        }
        if (!this->IsActive())
        {
            return false;
        }
        auto handler = new MainSocketSendHandler(this->mAddress, message);
        return this->mNetManager->PushEventHandler(handler);
    }

    bool TcpProxySession::SendMessageData(const char * message, size_t size)
    {
        if (message == nullptr || size <= 3)
        {
            return false;
        }
		if (!this->IsActive())
		{
			return false;
		}
		auto handler = new MainSocketSendHandler(this->mAddress, message, size);
        return this->mNetManager->PushEventHandler(handler);
    }

	void TcpProxySession::StartClose()
	{
		auto handler = new MainSocketCloseHandler(mAddress);
		this->mNetManager->PushEventHandler(handler);
#ifdef SOEASY_DEBUG
		SayNoDebugError("desctory session [" << this->mAddress << "]");
#endif
	}

	void TcpProxySession::StartConnect()
    {
        if (!this->IsNodeSession())
        {
            return;
        }
        this->mConnectCount++;
		auto handler = new MainSocketConnectHandler(mAddress, mName);
        this->mNetManager->PushEventHandler(handler);
    }

	void TcpProxySession::SetActive(bool active)
	{
		this->mIsActive = active;
		if (this->mIsActive)
		{
			this->mConnectCount = 0;
		}
	}
}
