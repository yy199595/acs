#include"SessionManager.h"
#include"NetWorkManager.h"
#include"ActionManager.h"

#include<CommonCore/Applocation.h>
namespace SoEasy
{
	shared_ptr<TcpClientSession> SessionManager::CreateTcpSession(SharedTcpSocket socket)
	{
		shared_ptr<TcpClientSession> tcpSession = std::make_shared<TcpClientSession>(this, socket);
		if (tcpSession != nullptr)
		{
			this->mNewSessionQueue.AddItem(tcpSession);
			return tcpSession;
		}
		return nullptr;
	}

	shared_ptr<TcpClientSession> SessionManager::CreateTcpSession(std::string name, std::string ip, unsigned short port)
	{
		shared_ptr<TcpClientSession> tcpSession = std::make_shared<TcpClientSession>(this, name, ip, port);
		if (tcpSession != nullptr)
		{
			tcpSession->StartConnect();
		}
		return tcpSession;
	}

	bool SessionManager::AddNewSession(shared_ptr<TcpClientSession> tcpSession)
	{
		if (!tcpSession)
		{
			return false;
		}
		if (!tcpSession->IsActive())
		{
			SayNoDebugError(tcpSession->GetAddress() << " is error add fail");
			return false;
		}
		this->mNewSessionQueue.AddItem(tcpSession);
		return true;
	}

	bool SessionManager::AddErrorSession(shared_ptr<TcpClientSession> tcpSession)
	{
		if (!tcpSession)
		{
			return false;
		}
		if (tcpSession->IsActive())
		{
			tcpSession->CloseSocket();
			return false;
		}
		this->mErrorSessionQueue.AddItem(tcpSession);
		return true;
	}

	void SessionManager::AddRecvMessage(shared_ptr<TcpClientSession> session, const char * message, size_t size)
	{
		if (session != nullptr)
		{
			const std::string & address = session->GetAddress();
			SharedNetPacket sharedPacket = std::make_shared<NetMessageBuffer>(address, message, size);
			mRecvMessageQueue.AddItem(sharedPacket);
		}
	}

	bool SessionManager::OnInit()
	{
		this->mNetWorkManager = this->GetManager<NetWorkManager>();
		this->mFunctionManager = this->GetManager<ActionManager>();
		SayNoAssertRetFalse_F(this->mNetWorkManager);
		SayNoAssertRetFalse_F(this->mFunctionManager);
		return true;
	}

	void SessionManager::OnRecvNewMessageAfter(const std::string & address, const char * msg, size_t size)
	{
		shared_ptr<TcpClientSession> pTcpSession = this->mNetWorkManager->GetSessionByAdress(address);
		SayNoAssertRet(pTcpSession, "not find session : " << address << " size = " << size);

		NetWorkPacket nNetMsgPackage;
		if (!nNetMsgPackage.ParseFromArray(msg, size))
		{
			SayNoDebugError("parse message pack fail : " << address << " size = " << size);
			return;
		}
		if (!nNetMsgPackage.func_name().empty())
		{
			const std::string & name = nNetMsgPackage.func_name();
			this->mFunctionManager->Call(pTcpSession, name, nNetMsgPackage);
		}
		else if (nNetMsgPackage.callback_id() != 0)
		{
			const long long id = nNetMsgPackage.callback_id();
			this->mFunctionManager->Call(pTcpSession, id, nNetMsgPackage);
		}
		else
		{
			std::string json;
			if (ProtocHelper::GetJsonString(nNetMsgPackage, json))
			{
				SayNoDebugError("unknow message : " << json);
			}
		}
	}

	void SessionManager::OnSystemUpdate()
	{
		shared_ptr<TcpClientSession> pTcpSession = nullptr;
		this->mNewSessionQueue.SwapQueueData();
		this->mErrorSessionQueue.SwapQueueData();

		while (this->mErrorSessionQueue.PopItem(pTcpSession))
		{
			if (!this->mNetWorkManager->RemoveTcpSession(pTcpSession))
			{
				SayNoDebugError("remove tcp session error : " << pTcpSession->GetAddress());
				continue;
			}
			this->OnSessionErrorAfter(pTcpSession);
			pTcpSession = nullptr;
		}

		while (this->mNewSessionQueue.PopItem(pTcpSession))
		{
			if (!this->mNetWorkManager->AddTcpSession(pTcpSession))
			{
				SayNoDebugError("add tcp session error : " << pTcpSession->GetAddress());
				continue;
			}
			this->OnSessionConnectAfter(pTcpSession);
			pTcpSession = nullptr;
		}

		SharedNetPacket pMessagePacket;
		this->mRecvMessageQueue.SwapQueueData();
		while (this->mRecvMessageQueue.PopItem(pMessagePacket))
		{
			const std::string & address = pMessagePacket->mAddress;
			const std::string & message = pMessagePacket->mCommandMsg;
			this->OnRecvNewMessageAfter(address, message.c_str(), message.size());
		}
	}
}