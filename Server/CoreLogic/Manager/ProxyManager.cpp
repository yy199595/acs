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
		SayNoAssertRetFalse_F(this->mRemoteActionManager = this->GetManager<RemoteActionManager>());
		this->mRemoteActionManager->SetRecvCallback(BIND_THIS_ACTION_2(ProxyManager::OnRecvServerMessage));
		return ListenerManager::OnInit();
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

	void ProxyManager::OnRecvNewMessageAfter(SharedTcpSession tcpSession, shared_ptr<NetWorkPacket> packet)
	{
		shared_ptr<GameObject> clientObject = this->GetClientObject(tcpSession->GetSocketId());
		if (clientObject != nullptr)//客户端发送过来的消息
		{
			const std::string & name = packet->func_name();
			const long long id = clientObject->GetGameObjectID();

			shared_ptr<RemoteActionProxy> actionProxy;
			if (this->mRemoteActionManager->GetActionProxy(name, actionProxy))
			{
				packet->set_operator_id(id);
				actionProxy->Invoke(packet);
				return;
			}
			packet->clear_func_name();
			packet->clear_operator_id();
			packet->clear_message_data();
			packet->set_error_code(XCode::CallFunctionNotExist);
			this->mNetWorkManager->SendMessageByAdress(tcpSession->GetAddress(), packet);
		}
		else
		{
			SessionManager::OnRecvNewMessageAfter(tcpSession, packet);
		}
	}

	shared_ptr<GameObject> ProxyManager::GetClientObject(const long long id)
	{
		auto iter = this->mClientObjectMap.find(id);
		return iter != this->mClientObjectMap.end() ? iter->second : nullptr;
	}

	bool ProxyManager::OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> messageData)
	{
		const long long playerId = messageData->operator_id();
		shared_ptr<GameObject> playerObject = this->GetClientObject(playerId);
		if (playerObject != nullptr)
		{
			const std::string & address = playerObject->GetBindAddress();
			this->mNetWorkManager->SendMessageByAdress(address, messageData);
			return true;
		}
		return false;
	}
}