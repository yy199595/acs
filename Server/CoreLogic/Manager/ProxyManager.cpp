#include"ProxyManager.h"
#include<Util/StringHelper.h>
#include<Util/NumberHelper.h>
#include<Core/TcpSessionListener.h>
#include<Manager/NetWorkManager.h>
namespace SoEasy
{
	bool ProxyManager::OnInit()
	{
		SayNoAssertRetFalse_F(ListenerManager::OnInit());
		return true;
	}

	void ProxyManager::OnSessionErrorAfter(SharedTcpSession tcpSession)
	{
		const long long id = tcpSession->GetSocketId();
		auto iter = this->mClientObjectMap.find(id);
		if (iter != this->mClientObjectMap.end())
		{
			this->mClientObjectMap.erase(iter);
		}
	}

	void ProxyManager::OnSessionConnectAfter(SharedTcpSession tcpSession)
	{	
		if (!tcpSession->IsContent())	//¿Í»§¶Ë
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