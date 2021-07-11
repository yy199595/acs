#include"ClientManager.h"
#include<Util/MathHelper.h>
#include<Util/StringHelper.h>

#include<Protocol/db.pb.h>
#include<Pool/ObjectPool.h>

#include<Manager/ActionManager.h>

#include<Other/ObjectFactory.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
namespace Client
{
	bool ClientManager::OnInit()
	{	
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", this->mAddress));
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(this->mAddress, this->mConnectIp, this->mConnectPort));
		
		return NetProxyManager::OnInit();;
	}

	void ClientManager::OnInitComplete()
	{
		this->ConnectByAddress(this->mAddress, "GateServer");
		this->mCoroutineManager->Start(BIND_THIS_ACTION_0(ClientManager::InvokeAction));
	}

	void ClientManager::OnFrameUpdate(float t)
	{
		if (!this->mWaitSendMessages.empty())
		{
			TcpProxySession * tcpSession = this->GetProxySession(this->mAddress);
			if (tcpSession != nullptr)
			{
				PB::NetWorkPacket * messageData = this->mWaitSendMessages.front();
				tcpSession->SendMessageData(messageData);
				this->mWaitSendMessages.pop();
			}						
		}
	}

	XCode ClientManager::Notice(const std::string & service, const std::string & method)
	{
		return XCode();
	}

	XCode ClientManager::Notice(const std::string & service, const std::string & method, const Message & request)
	{
		return XCode();
	}

	XCode ClientManager::Invoke(const std::string & service, const std::string & method)
	{
		return XCode();
	}

	XCode ClientManager::Invoke(const std::string & service, const std::string & method, const Message & request)
	{
		return XCode();
	}

	XCode ClientManager::Call(const std::string & service, const std::string & method, Message & response)
	{
		PB::NetWorkPacket * messageData = GnetPacketPool.Create();
		messageData->set_service(service);
		messageData->set_method(method);

		

		return XCode();
	}

	XCode ClientManager::Call(const std::string & service, const std::string & method, const Message & request, Message & response)
	{
		PB::NetWorkPacket * messageData = GnetPacketPool.Create();
		ActionManager * pActionManager = this->GetManager<ActionManager>();
		if (messageData == nullptr)
		{
			return XCode::Failure;
		}
		messageData->set_method(method);
		messageData->set_service(service);
		messageData->set_messagedata(request.SerializeAsString());

		shared_ptr<NetWorkWaitCorAction> rpcCallback = NetWorkWaitCorAction::Create(this->mCoroutineManager);
		if (rpcCallback != nullptr)
		{
#ifdef SOEASY_DEBUG
			rpcCallback->mMethod = method;
			rpcCallback->mService = service;
#endif
			messageData->set_rpcid(pActionManager->AddCallback(rpcCallback));
		}
		this->mWaitSendMessages.push(messageData);
		this->mCoroutineManager->YieldReturn();
		response.ParseFromString(rpcCallback->GetMsgData());
		return rpcCallback->GetCode();

	}

	void ClientManager::InvokeAction()
	{

		s2s::MysqlQuery_Request requestData;
		s2s::MysqlQuery_Response responseData;

		db::UserAccountData userAccountData;
		
		userAccountData.set_userid(13716061995);

		requestData.set_protocolname(userAccountData.GetTypeName());
		requestData.set_protocolmessage(userAccountData.SerializeAsString());
		
		while (true)
		{
			XCode code = this->Call("MysqlProxy", "QueryData", requestData, responseData);
			if (code != XCode::Successful)
			{
				const std::string & err = responseData.errotstr();
				SayNoDebugError(err);
			}
			for (int index = 0; index < responseData.querydatas_size(); index++)
			{
				const std::string & value = responseData.querydatas(index);
				userAccountData.ParseFromString(value);
				SayNoDebugLogProtocBuf(userAccountData);
			}
		}
	}
}
