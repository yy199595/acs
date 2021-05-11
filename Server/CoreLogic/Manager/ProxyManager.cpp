#include"ProxyManager.h"
#include<Util/StringHelper.h>
#include<Util/NumberHelper.h>
#include<Core/TcpSessionListener.h>
#include<Manager/NetWorkManager.h>
#include<Manager/RemoteActionManager.h>
namespace SoEasy
{
	bool ProxyManager::OnInit()
	{
		if (!SessionManager::OnInit())
		{
			return false;
		}
		std::string address;
		if (!this->GetConfig().GetValue("ProxyAddress", address))
		{
			SayNoDebugFatal("not find field 'ProxyAddress'");
			return false;
		}
		if (!this->ParseAddress(address, this->mListenIp, this->mListenPort))
		{
			SayNoDebugFatal("parse ProxyAddress fail");
			return false;
		}
		this->mRemoteActionManager = this->GetManager<RemoteActionManager>();
		SayNoAssertRetFalse_F(this->mRemoteActionManager);		
		this->mTpcListener = make_shared<TcpSessionListener>(this, this->mListenPort);
		if (!this->mTpcListener->InitListener())
		{
			return false;
		 }
		this->mTpcListener->StartAcceptConnect();
		return true;
	}

	void ProxyManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		const long long id = tcpSession->GetSocketId();
		auto iter = this->mClientObjectMap.find(id);
		if (iter != this->mClientObjectMap.end())
		{
			this->mClientObjectMap.erase(iter);
		}
	}

	void ProxyManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{	
		if (!tcpSession->IsContent())	//客户端
		{
			const std::string & address = tcpSession->GetAddress();
			const long long clientObjectId = tcpSession->GetSocketId();
			shared_ptr<GameObject> clientObject = make_shared<GameObject>(clientObjectId, address);
			this->mClientObjectMap.emplace(clientObjectId, clientObject);
		}
		else
		{
			
		}
	}

	void ProxyManager::OnRecvNewMessageAfter(const std::string & address, const char * msg, size_t size)
	{
		shared_ptr<TcpClientSession> clientSession = this->mNetWorkManager->GetTcpSession(address);

		SayNoAssertRet_F(clientSession);

		shared_ptr<NetWorkPacket> netPacket = make_shared<NetWorkPacket>();
		SayNoAssertRet_F(netPacket->ParseFromArray(msg, size));

		shared_ptr<GameObject> clientObject = this->GetClientObject(clientSession->GetSocketId());
		if (clientObject != nullptr)//客户端发送过来的消息
		{
			const std::string & name = netPacket->func_name();
			const long long id = clientObject->GetGameObjectID();

			shared_ptr<RemoteActionProxy> actionProxy;
			if (this->mRemoteActionManager->GetActionProxy(name, actionProxy))
			{
				actionProxy->Invoke(netPacket);
				return;
			}
			netPacket->clear_func_name();
			netPacket->clear_operator_id();
			netPacket->clear_message_data();
			netPacket->set_error_code(XCode::CallFunctionNotExist);
			this->mNetWorkManager->SendMessageByAdress(address, netPacket);
		}
	}

	shared_ptr<GameObject> ProxyManager::GetClientObject(const long long id)
	{
		auto iter = this->mClientObjectMap.find(id);
		return iter != this->mClientObjectMap.end() ? iter->second : nullptr;
	}

	void ProxyManager::OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> messageData)
	{
		const long long playerId = messageData->operator_id();
		shared_ptr<GameObject> playerObject = this->GetClientObject(playerId);
		if (playerObject != nullptr)
		{
			const std::string & address = playerObject->GetBindAddress();
			this->mNetWorkManager->SendMessageByAdress(address, messageData);
		}
	}
}