#include"TcpProxySession.h"
#include<Core/Applocation.h>
#include<Pool/ObjectPool.h>
#include<Manager/NetProxyManager.h>
#include<Manager/NetSessionManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{

	TcpProxySession::TcpProxySession(const std::string & address)
	{	
		this->mAddress = address;
		this->mSessionType = SessionClient;
		Applocation * app = Applocation::Get();
		SayNoAssertRet_F(this->mCorManager = app->GetManager<CoroutineManager>());
		SayNoAssertRet_F(this->mNetManager = app->GetManager<NetSessionManager>());
		SayNoAssertRet_F(this->mNetProxyManager = app->GetManager<NetProxyManager>());

	}

	TcpProxySession::TcpProxySession(const std::string & name, const std::string & address)
	{
		this->mName = name;
		this->mAddress = address;
		this->mSessionType = SessionNode;
		Applocation * app = Applocation::Get();
		SayNoAssertRet_F(this->mCorManager = app->GetManager<CoroutineManager>());
		SayNoAssertRet_F(this->mNetManager = app->GetManager<NetSessionManager>());
		SayNoAssertRet_F(this->mNetProxyManager = app->GetManager<NetProxyManager>());
	}

	bool TcpProxySession::SendMessageData(PB::NetWorkPacket * messageData)
	{
		if (messageData == nullptr)
		{
			return false;
		}
		return this->mNetProxyManager->SendMsgByAddress(this->mAddress, messageData);
	}

	bool TcpProxySession::Notice(const std::string & service, const std::string & method)
	{
		PB::NetWorkPacket * messageData = NetPacketPool.Create();
		if (messageData == nullptr)
		{
			return XCode::Failure;
		}
		messageData->set_method(method);
		messageData->set_service(service);
		return this->mNetProxyManager->SendMsgByAddress(this->mAddress, messageData);
	}

	bool TcpProxySession::Notice(const std::string & service, const std::string & method, const Message & request)
	{
		PB::NetWorkPacket * messageData = NetPacketPool.Create();
		if (messageData == nullptr)
		{
			return XCode::Failure;
		}
		messageData->set_method(method);
		messageData->set_service(service);
		messageData->set_messagedata(request.SerializeAsString());
		return this->mNetProxyManager->SendMsgByAddress(this->mAddress, messageData);
	}
	XCode TcpProxySession::Invoke(const std::string & service, const std::string & method)
	{
		return XCode();
	}
}
