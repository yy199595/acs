#include"NetWorkManager.h"
#include"ScriptManager.h"
#include"ActionManager.h"
#include"Manager.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<Manager/ScriptManager.h>
#include<Script/LuaFunction.h>
#include<Manager/ActionManager.h>
#include<NetWork/RemoteActionProxy.h>
namespace SoEasy
{
	NetWorkManager::NetWorkManager() : mSessionContext(nullptr)
	{
		
	}

	bool NetWorkManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mSessionContext = this->GetApp()->GetAsioContextPtr());
		SayNoAssertRetFalse_F(this->mSessionContext = this->GetApp()->GetAsioContextPtr());
		
		SayNoAssertRetFalse_F(this->mLocalActionManager = this->GetManager<ActionManager>());
		return true;
	}

	bool NetWorkManager::RemoveTcpSession(const std::string & address)
	{
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{			
			shared_ptr<TcpClientSession> tcpSession = iter->second;
			const long long socketId = tcpSession->GetSocketId();
			this->mSessionAdressMap.erase(iter);
			SayNoDebugWarning("remove session " << tcpSession->GetSessionName() << " adress:" << tcpSession->GetAddress());
			return true;
		}
		return false;
	}

	bool NetWorkManager::RemoveTcpSession(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		return this->RemoveTcpSession(address);
	}

	XCode NetWorkManager::SendMessageByAdress(const std::string & address, const SharedPacket & returnPackage)
	{
		return this->SendMessageByAdress(address, *returnPackage);
	}

	bool NetWorkManager::AddTcpSession(shared_ptr<TcpClientSession> tcpSession)
	{
		SayNoAssertRetFalse_F(tcpSession);
		const std::string & address = tcpSession->GetAddress();
		auto iter = this->mSessionAdressMap.find(address);
		if (iter == this->mSessionAdressMap.end())
		{
			if (tcpSession->StartReceiveMsg())
			{
				this->mSessionAdressMap.emplace(address, tcpSession);
				SayNoDebugError("add new session " << address);
				return true;
			}
		}
		return false;
	}

	bool NetWorkManager::CloseTcpSession(const std::string & address)
	{
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			iter->second->StartClose();
			return true;
		}
		return false;
	}

	bool NetWorkManager::CloseTcpSession(shared_ptr<TcpClientSession> tcpSession)
	{
		const std::string & address = tcpSession->GetAddress();
		return this->CloseTcpSession(address);
	}

	XCode NetWorkManager::SendMessageByAdress(const std::string & address, const NetWorkPacket & returnPackage)
	{
		shared_ptr<TcpClientSession> pSession = this->GetTcpSession(address);

		if (pSession == nullptr)
		{
			SayNoDebugError("not find session " << address);
			return XCode::SessionIsNull;
		}
		
		char * bufferStartPos = this->mSendSharedBuffer + sizeof(unsigned int);
		if (!returnPackage.SerializeToArray(bufferStartPos, ASIO_TCP_SEND_MAX_COUNT))
		{
			SayNoDebugError("Serialize Fail : " << returnPackage.method());
			return XCode::SerializationFailure;
		}
		size_t size = returnPackage.ByteSizeLong();
		size_t length = size + sizeof(unsigned int);
		memcpy(this->mSendSharedBuffer, &size, sizeof(unsigned int));
		SayNoDebugLog("call " << returnPackage.service() << "." << returnPackage.method());
		shared_ptr<string> sendMessage = make_shared<string>(this->mSendSharedBuffer, length);		
		return pSession->SendPackage(sendMessage) ? XCode::Successful : XCode::SendMessageFail;
	}

	shared_ptr<TcpClientSession> NetWorkManager::GetTcpSession(const std::string & adress)
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

	void NetWorkManager::OnDestory()
	{
		
	}
}