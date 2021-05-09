#include"NetWorkManager.h"
#include"ScriptManager.h"
#include"ActionManager.h"
#include"Manager.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<NetWork/RemoteScheduler.h>
#include<Manager/ScriptManager.h>
#include<Script/LuaType/LuaFunction.h>
#include<Manager/ActionQueryManager.h>
#include<NetWork/ActionAddressProxy.h>
namespace SoEasy
{
	NetWorkManager::NetWorkManager() : mSessionContext(nullptr)
	{
		
	}

	bool NetWorkManager::OnInit()
	{
		this->mSessionContext = this->GetApp()->GetAsioContextPtr();
		this->mActionQueryManager = this->GetManager<ActionQueryManager>();

		SayNoAssertRetFalse_F(this->mSessionContext);
		//SayNoAssertRetFalse_F(this->mActionQueryManager);
		return SessionManager::OnInit();
	}

	bool NetWorkManager::RemoveTcpSession(const std::string & address)
	{
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{			
			shared_ptr<TcpClientSession> tcpSession = iter->second;
			SayNoDebugWarning("remove session " << tcpSession->GetSessionName() << " adress:" << tcpSession->GetAddress());
			this->mSessionAdressMap.erase(iter);
			return true;
		}
		return false;
	}

	bool NetWorkManager::RemoveTcpSession(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		return this->RemoveTcpSession(address);
	}

	bool NetWorkManager::AddTcpSession(shared_ptr<TcpClientSession> tcpSession)
	{
		SayNoAssertRetFalse_F(tcpSession);
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mSessionAdressMap.find(address);
		if (iter == this->mSessionAdressMap.end())
		{
			this->mSessionLock.lock();
			tcpSession->StartReceiveMsg();
			this->mSessionLock.unlock();
			this->mSessionAdressMap.insert(std::make_pair(address, tcpSession));
			SayNoDebugError("add new session " << address);
			return true;
		}
		return false;
	}

	bool NetWorkManager::CloseTcpSession(const std::string & address)
	{
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			this->mSessionLock.lock();
			iter->second->CloseSocket();
			this->mSessionLock.unlock();
			return true;
		}
		return false;
	}

	bool NetWorkManager::CloseTcpSession(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		return this->CloseTcpSession(address);
	}

	void NetWorkManager::OnFrameUpdateAfter() 
	{ 
		
	}

	void NetWorkManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void NetWorkManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		
	}

	bool NetWorkManager::SendMessageByAdress(const std::string & address, shared_ptr<NetWorkPacket> returnPackage)
	{
		shared_ptr<TcpClientSession> pSession = this->GetSessionByAdress(address);

		if (pSession == nullptr)
		{
			SayNoDebugError("not find session " << address);
			return false;
		}
		
		char * bufferStartPos = this->mSendSharedBuffer + sizeof(unsigned int);
		if (!returnPackage->SerializeToArray(bufferStartPos, ASIO_TCP_SEND_MAX_COUNT))
		{
			SayNoDebugError("Serialize Fail : " << returnPackage->func_name());
			return false;
		}
		size_t size = returnPackage->ByteSizeLong();
		size_t length = size + sizeof(unsigned int);
		memcpy(this->mSendSharedBuffer, &size, sizeof(unsigned int));
		shared_ptr<string> sendMessage = make_shared<string>(this->mSendSharedBuffer, length);
		
		this->mSessionContext->post([pSession, sendMessage]()
		{
			const char * data = sendMessage->c_str();
			const size_t size = sendMessage->size();
			pSession->SendPackage(data, size);
		});
		return true;
	}

	shared_ptr<TcpClientSession> NetWorkManager::GetSessionByAdress(const std::string & adress)
	{
		auto iter = this->mSessionAdressMap.find(adress);
		if (iter != this->mSessionAdressMap.end())
		{
			shared_ptr<TcpClientSession> session = iter->second;
			if (!session->IsActive())
			{
				this->mSessionAdressMap.erase(iter);
				return nullptr;
			}
			return session;
		}
		return nullptr;
	}

	bool NetWorkManager::SendMessageByName(const std::string & func, shared_ptr<NetWorkPacket> returnPackage)
	{
		long long operId = returnPackage->operator_id();
		ActionAddressProxy * actionProxy = this->mActionQueryManager->GetActionProxy(func, operId);
		if (actionProxy == nullptr)
		{
			SayNoDebugError(func << "  method does not exist");
			return false;
		}
		return actionProxy->CallAction(returnPackage);
	}
	
	void NetWorkManager::OnDestory()
	{
		
	}
}