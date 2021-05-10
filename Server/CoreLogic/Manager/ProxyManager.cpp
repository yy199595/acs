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
		this->mActionQueryManager = this->GetManager<RemoteActionManager>();
		SayNoAssertRetFalse_F(this->mActionQueryManager);
		this->mTpcListener = make_shared<TcpSessionListener>(this, this->mListenPort);
		if (!this->mTpcListener->InitListener())
		{
			return false;
		 }
		this->mTpcListener->StartAcceptConnect();
		return SessionManager::OnInit();
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
		shared_ptr<TcpClientSession> clientSession = this->mNetWorkManager->GetSessionByAdress(address);

		SayNoAssertRet_F(clientSession);

		shared_ptr<NetWorkPacket> netPacket = make_shared<NetWorkPacket>();
		SayNoAssertRet_F(netPacket->ParseFromArray(msg, size));

		shared_ptr<GameObject> clientObject = this->GetClientObject(clientSession->GetSocketId());
		if (clientObject != nullptr)//客户端发送过来的消息
		{
			const std::string & name = netPacket->func_name();
			const long long id = clientObject->GetGameObjectID();
			RemoteActionProxy * actionProxy = this->mActionQueryManager->GetActionProxy(name, id);
			if (actionProxy == nullptr)
			{
				netPacket->clear_func_name();
				netPacket->clear_operator_id();
				netPacket->clear_message_data();
				netPacket->set_error_code(XCode::CallFunctionNotExist);
				this->mNetWorkManager->SendMessageByAdress(address, netPacket);
				return;
			}
			netPacket->set_operator_id(id);
			XCode code = actionProxy->CallAction(netPacket);
			if (code == XCode::SessionIsNull)
			{
				actionProxy->StartConnect(this);
			}
		}
		else if(netPacket->operator_id() != 0) //转发给客户端
		{
			clientObject = this->GetClientObject(netPacket->operator_id());
			if (clientObject != nullptr)
			{
				const std::string & address = clientObject->GetBindAddress();
				this->mNetWorkManager->SendMessageByAdress(address, netPacket);
			}
		}
		else  //调用本机方法
		{

		}
	}

	shared_ptr<GameObject> ProxyManager::GetClientObject(const long long id)
	{
		auto iter = this->mClientObjectMap.find(id);
		return iter != this->mClientObjectMap.end() ? iter->second : nullptr;
	}
}