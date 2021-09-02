#include "TcpProxySession.h"
#include <Core/App.h>
#include <Scene/SceneNetProxyComponent.h>
#include <Scene/SceneSessionComponent.h>
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{

    TcpProxySession::TcpProxySession(const std::string &address)
    {
		this->mIsActive = true;
        this->mConnectCount = 0;
        this->mAddress = address;
        this->mSessionType = SessionClient;    
        SayNoAssertRet_F(this->mNetManager = Scene::GetComponent<SceneSessionComponent>());
    }

    TcpProxySession::TcpProxySession(const std::string &name, const std::string &address)
    {
        this->mName = name;
        this->mConnectCount = 0;
		this->mIsActive = false;
        this->mAddress = address;
        this->mSessionType = SessionNode;
		SayNoAssertRet_F(this->mNetManager = Scene::GetComponent<SceneSessionComponent>());
    }

    TcpProxySession::~TcpProxySession()
    {    
		MainSocketCloseHandler * handler = new MainSocketCloseHandler(mAddress);
        this->mNetManager->PushEventHandler(handler);
#ifdef SOEASY_DEBUG
        SayNoDebugError("desctory session [" << this->mAddress << "]");
#endif
    }

    bool TcpProxySession::SendMessageData(PacketMapper *messageData)
    {
        if (messageData == nullptr)
        {
            return false;
        }
		if (this->IsActive() == false)
		{
			return false;
		}
		MainSocketSendHandler * handler = new MainSocketSendHandler(messageData);
        return this->mNetManager->PushEventHandler(handler);
    }

	void TcpProxySession::StartColse()
	{
		MainSocketCloseHandler * handler = new MainSocketCloseHandler(mAddress);
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
		MainSocketConnectHandler * handler = new MainSocketConnectHandler(mAddress, mName);
        this->mNetManager->PushEventHandler(handler);
    }

    bool TcpProxySession::Notice(const std::string &service, const std::string &method)
    {
        if (service.empty() || method.empty())
        {
            return false;
        }
		const std::string & address = this->GetAddress();
        PacketMapper *messageData = PacketMapper::Create(address, S2S_NOTICE, service, method);
        if (messageData == nullptr)
        {
            SayNoDebugError("not find method " << service << "." << method);
            return XCode::Failure;
        }
        return this->SendMessageData(messageData);
    }

    bool TcpProxySession::Notice(const std::string &service, const std::string &method, const Message &request)
    {
        if (service.empty() || method.empty())
        {
            return false;
        }
		const std::string & address = this->GetAddress();
        PacketMapper *messageData = PacketMapper::Create(address, S2S_NOTICE, service, method);
        if (messageData == nullptr)
        {
            SayNoDebugError("not find method " << service << "." << method);
            return XCode::Failure;
        }
		messageData->SetMessage(request);      
        return this->SendMessageData(messageData);
    }
}
