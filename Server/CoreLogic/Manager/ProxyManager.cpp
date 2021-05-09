#include"ProxyManager.h"
#include<Util/StringHelper.h>
#include<Util/NumberHelper.h>
#include<Core/TcpSessionListener.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionQueryManager.h>
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
		this->mActionQueryManager = this->GetManager<ActionQueryManager>();
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
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mClientSessionMap.find(address);
		auto iter1 = this->mClientSessionIdMap.find(address);
		if (iter != this->mClientSessionMap.end() && iter1 != this->mClientSessionIdMap.end())
		{
			this->mClientSessionMap.erase(iter);
			this->mClientSessionIdMap.erase(iter1);
		}
	}

	void ProxyManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{	
		const std::string & address = tcpSession->GetAddress();
		if (!tcpSession->IsContent())	//客户端
		{
			auto iter = this->mClientSessionIdMap.find(address);
			if (iter == this->mClientSessionIdMap.end())
			{
				long long id = NumberHelper::Create();
				this->mClientSessionIdMap.emplace(address, id);
				this->mClientSessionMap.emplace(address, tcpSession);
			}
		}
		else
		{
			
		}
	}

	void ProxyManager::OnRecvNewMessageAfter(const std::string & address, const char * msg, size_t size)
	{
		shared_ptr<TcpClientSession> tcpSession = this->mNetWorkManager->GetSessionByAdress(address);	
		shared_ptr<NetWorkPacket> netPacket = make_shared<NetWorkPacket>();
		SayNoAssertRet_F(netPacket->ParseFromArray(msg, size));
		shared_ptr<TcpClientSession> clientSession = this->GetClientSession(address);
		if (clientSession != nullptr)//客户端调用服务器消息
		{
			const std::string & name = netPacket->func_name();
			long long id = this->mClientSessionIdMap[address];
			ActionAddressProxy * actionProxy = this->mActionQueryManager->GetActionProxy(name, id);
			if (actionProxy == nullptr)
			{
				netPacket->clear_func_name();
				netPacket->clear_operator_id();
				netPacket->clear_message_data();
				netPacket->set_error_code(XCode::CallFunctionNotExist);
				this->mNetWorkManager->SendMessageByAdress(address, netPacket);
				return;
			}
			actionProxy->CallAction(netPacket);
		}
		else
		{
			SayNoDebugWarning("-------------------------");
		}
	}

	shared_ptr<TcpClientSession> ProxyManager::GetClientSession(const std::string & address)
	{
		auto iter = this->mClientSessionMap.find(address);
		return iter != this->mClientSessionMap.end() ? iter->second : nullptr;
	}
}