#include"NetWorkManager.h"
#include"ScriptManager.h"
#include"ActionManager.h"
#include"CommandManager.h"
#include<CommonCore/Applocation.h>
#include<CommonUtil/StringHelper.h>
#include<CommonNetWork/TcpClientSession.h>
#include<CommonNetWork/RemoteScheduler.h>
#include<CommonManager/ScriptManager.h>
#include<CommonScript/LuaType/LuaFunction.h>

namespace SoEasy
{
	NetWorkManager::NetWorkManager() : mSessionContext(nullptr)
	{
		this->mBindLuaTalbe = nullptr;
	}

	bool NetWorkManager::OnInit()
	{
		this->mSessionContext = this->GetApp()->GetAsioContextPtr();
		REGISTER_FUNCTION_1(NetWorkManager::UpdateAction, PB::AreaActionInfo);
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
		
	}

	void NetWorkManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void NetWorkManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mWaitSendMessage.find(address);
		if (iter != this->mWaitSendMessage.end())
		{
			while (!iter->second.empty())
			{
				shared_ptr<NetWorkPacket> data = iter->second.front();
				this->SendMessageByAdress(address, data);
				iter->second.pop();
			}
		}
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

	bool NetWorkManager::StartConnect(const std::string name, const std::string & address)
	{
		string ip;
		unsigned short port;
		if (!StringHelper::ParseIpAddress(address, ip, port))
		{
			return false;
		}
		shared_ptr<TcpClientSession> session = make_shared<TcpClientSession>(this, name, ip, port);
		this->mOnConnectSessionMap.emplace(session->GetAddress(), session);
		return session->StartConnect();
	}

	XCode NetWorkManager::UpdateAction(shared_ptr<TcpClientSession> session, long long id, const PB::AreaActionInfo & actionInfos)
	{
		this->mRemoteAddressMap.clear();
		for (int index = 0; index < actionInfos.action_infos_size(); index++)
		{
			std::vector<std::string> addressVector;
			const AreaActionInfo_ActionInfo & actionInfo = actionInfos.action_infos(index);
			for (int i = 0; i < actionInfo.action_address_size(); i++)
			{
				const std::string & address = actionInfo.action_address(i);
				addressVector.emplace_back(address);
			}
			const std::string & name = actionInfo.action_name();
			this->mRemoteAddressMap.emplace(name, addressVector);
		}
		return XCode::Successful;
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
		auto iter = this->mRemoteAddressMap.find(func);
		if (iter == this->mRemoteAddressMap.end())
		{
			SayNoDebugError(func << "method does not exist");
			return false;
		}
		long long operId = returnPackage->operator_id();
		std::vector<std::string> & addressVector = iter->second;
		const std::string & address = addressVector[operId % addressVector.size()];

		shared_ptr<TcpClientSession> pSession = this->GetSessionByAdress(address);
		if (pSession != nullptr)
		{
			return this->SendMessageByAdress(address, returnPackage);	
		}

		auto iter1 = this->mOnConnectSessionMap.find(address);
		if (iter1 == this->mOnConnectSessionMap.end())
		{
			std::string ip;
			unsigned short port;
			
			auto iter2 = mWaitSendMessage.find(address);
			if (iter2 == mWaitSendMessage.end())
			{
				std::queue<shared_ptr<NetWorkPacket>> sendQueue;
				this->mWaitSendMessage.emplace(address, sendQueue);
			}
			this->mWaitSendMessage[address].push(returnPackage);
			return this->StartConnect("ActionSession", address);
		}
		return true;
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