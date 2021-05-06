#include"NetWorkManager.h"
#include"ScriptManager.h"
#include"ActionManager.h"
#include"CommandManager.h"
#include<CommonCore/Applocation.h>

#include<CommonNetWork/TcpClientSession.h>
#include<CommonNetWork/RemoteScheduler.h>
#include<CommonManager/ScriptManager.h>
#include<CommonScript/LuaType/LuaFunction.h>

namespace SoEasy
{
	NetWorkManager::NetWorkManager() : Manager(0)
	{
		this->mBindLuaTalbe = nullptr;
	}

	bool NetWorkManager::OnInit()
	{
		return true;
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
		while (!this->mSendMessageQueue.empty())
		{
			SharedNetPacket sharedPacket = this->mSendMessageQueue.front();
			this->mSendMessageQueue.pop();
			const std::string & address = sharedPacket->mAddress;
			const std::string & message = sharedPacket->mCommandMsg;
			shared_ptr<TcpClientSession> tcpSession = this->GetSessionByAdress(address);
			if (tcpSession != nullptr)
			{
				this->mSessionLock.lock();
				tcpSession->SendPackage(message);
				this->mSessionLock.unlock();
			}
		}
	}

	bool NetWorkManager::SendMessageByAdress(const std::string & address, const NetWorkPacket & returnPackage)
	{
		shared_ptr<TcpClientSession> pSession = this->GetSessionByAdress(address);
		SayNoAssertRetVal(pSession, "not find address : " << address, false);

		char * bufferStartPos = this->mSendSharedBuffer + sizeof(unsigned int);
		if (!returnPackage.SerializeToArray(bufferStartPos, ASIO_TCP_SEND_MAX_COUNT))
		{
			SayNoDebugError("Serialize Fail : " << returnPackage.func_name());
			return false;
		}
		size_t size = returnPackage.ByteSizeLong();
		size_t length = size + sizeof(unsigned int);
		memcpy(this->mSendSharedBuffer, &size, sizeof(unsigned int));
		SharedNetPacket sharedPacket = std::make_shared<NetMessageBuffer>(address, this->mSendSharedBuffer, length);
		this->mSendMessageQueue.push(sharedPacket);
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

	void NetWorkManager::OnLoadLuaComplete(lua_State * luaEnv)
	{
		if (this->mBindLuaTalbe != nullptr)
		{
			delete this->mBindLuaTalbe;
			this->mBindLuaTalbe = nullptr;
		}
		this->mBindLuaTalbe = LuaTable::Create(luaEnv, this->GetTypeName());
		if (this->mBindLuaTalbe != nullptr)
		{
			SayNoDebugLog("reference lua table " << this->GetTypeName() << " successful");
		}
	}
	
	void NetWorkManager::OnDestory()
	{
		
	}
}