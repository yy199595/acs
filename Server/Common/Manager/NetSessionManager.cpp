#include"NetSessionManager.h"
#include"ScriptManager.h"
#include"ActionManager.h"
#include"Manager.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<NetWork/TcpClientSession.h>
#include<Manager/ListenerManager.h>
#include<Manager/ActionManager.h>
#include<Manager/NetProxyManager.h>
namespace SoEasy
{
	NetSessionManager::NetSessionManager()
	{
		this->mIsClose = false;
		this->mNetThread = nullptr;
	}

	bool NetSessionManager::OnInit()
	{		
		SayNoAssertRetFalse_F(this->mNetProxyManager = this->GetManager<NetProxyManager>());
		SayNoAssertRetFalse_F(this->mLocalActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mListenerManager = this->GetManager<ListenerManager>());
		return true;
	}

	void NetSessionManager::OnConnectSuccess(SharedTcpSession session)
	{
#ifdef _DEBUG
		SayNoAssertRet_F(this->IsInNetThead());
#endif
		const std::string & address = session->GetAddress();
		const std::string & name = session->GetSessionName();
		Net2MainEvent * eve = new Net2MainEvent(SocketConnectSuc, address, name);
		this->mNetProxyManager->AddNetSessionEvent(eve);
	}

	void NetSessionManager::OnSessionError(SharedTcpSession session, Net2MainEventType type)
	{
#ifdef _DEBUG
		SayNoAssertRet_F(this->IsInNetThead());
#endif
		const std::string & address = session->GetAddress();
		const std::string & name = session->GetSessionName();
		Net2MainEvent * eve = new Net2MainEvent(type, address, name);
		this->mNetProxyManager->AddNetSessionEvent(eve);
	}

	bool NetSessionManager::OnRecvMessage(SharedTcpSession session, const char * message, const size_t size)
	{
#ifdef _DEBUG
		SayNoAssertRetFalse_F(this->IsInNetThead());
#endif
		PB::NetWorkPacket * msgData = NetPacketPool.Create();
		if (!msgData->ParseFromArray(message, size))
		{
			NetPacketPool.Destory(msgData);
			return false;
		}
		const std::string & address = session->GetAddress();
		const std::string & name = session->GetSessionName();
		Net2MainEvent * eve = new Net2MainEvent(SocketReceiveData, address, name, msgData);
		return this->mNetProxyManager->AddNetSessionEvent(eve);
	}

	bool NetSessionManager::OnSendMessageError(SharedTcpSession session, const char * message, const size_t size)
	{
#ifdef _DEBUG
		SayNoAssertRetFalse_F(this->IsInNetThead());
#endif
		if (message == nullptr || size == 0)
		{
			return false;
		}

		PB::NetWorkPacket * msgData = NetPacketPool.Create();
		if (!msgData->ParseFromArray(message, size))
		{
			NetPacketPool.Destory(msgData);
			return false;
		}
		const std::string & address = session->GetAddress();
		const std::string & name = session->GetSessionName();
		Net2MainEvent * eve = new Net2MainEvent(SocketSendMsgFail, address, name, msgData);
		return this->mNetProxyManager->AddNetSessionEvent(eve);
	}

	bool NetSessionManager::AddNetSessionEvent(Main2NetEvent * eve)
	{
#ifdef _DEBUG
		SayNoAssertRetFalse_F(!this->IsInNetThead());
#endif
		if (eve->GetEventType() == SocketSendMsgEvent)
		{
			if (eve->GetMsgData() == nullptr)
			{
				return false;
			}
		}
		this->mNetEventQueue.AddItem(eve);
		return true;
	}

	shared_ptr<TcpClientSession> NetSessionManager::Create(shared_ptr<AsioTcpSocket> socket)
	{
#ifdef _DEBUG
		SayNoAssertRetNull_F(this->IsInNetThead());
#endif
		SharedTcpSession session = make_shared<TcpClientSession>(this, socket);
		if (this->mSessionAdressMap.find(session->GetAddress()) != this->mSessionAdressMap.end())
		{
			session->StartReceiveMsg();
			this->mSessionAdressMap.emplace(session->GetAddress(), session);
			Net2MainEvent * eve = new Net2MainEvent(SocketNewConnect, session->GetAddress());
			this->mNetProxyManager->AddNetSessionEvent(eve);
			return session;
		}
		return nullptr;
	}

	shared_ptr<TcpClientSession> NetSessionManager::Create(const std::string & name, const std::string & address)
	{
#ifdef _DEBUG
		SayNoAssertRetNull_F(this->IsInNetThead());
#endif
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			return iter->second;
		}
		std::string ip;
		unsigned short port;
		if (StringHelper::ParseIpAddress(address, ip, port))
		{
			SharedTcpSession session = make_shared<TcpClientSession>(this, name, ip, port);
			this->mSessionAdressMap.emplace(address, session);
			return session;
		}
		return nullptr;
	}

	void NetSessionManager::OnDestory()
	{
		
	}
	void NetSessionManager::OnInitComplete()
	{
		this->mNetThread = new std::thread(BIND_THIS_ACTION_0(NetSessionManager::NetUpdate));
		this->mNetThread->detach();
	}
	
	void NetSessionManager::NetUpdate()
	{
		Main2NetEvent * sessionEvent = nullptr;
		auto sleep = std::chrono::milliseconds(1);
		this->mNetThreadId = std::this_thread::get_id();
		this->mListenerManager->StartAccept();
		while (this->mIsClose == false)
		{
			asio::error_code code;
			this->mAsioContext.poll(code);
			if (code)
			{
				SayNoDebugError(code.message());
			}
			this->mNetEventQueue.SwapQueueData(); //处理主线程过来的数据
			while (this->mNetEventQueue.PopItem(sessionEvent))
			{
				this->HandlerMainThreadEvent(sessionEvent);
				delete sessionEvent;
			}
			std::this_thread::sleep_for(sleep);			
		}
	}
	bool NetSessionManager::IsInNetThead()
	{
		return this->mNetThreadId == std::this_thread::get_id();
	}

	void NetSessionManager::HandlerMainThreadEvent(Main2NetEvent * eve) //处理主线程过来的事件
	{
		if (eve == nullptr) return;
		const std::string & address = eve->GetAddress();
		if (eve->GetEventType() == SocketDectoryEvent)
		{
			this->DescorySession(address);
		}
		else if (eve->GetEventType() == SocketConnectEvent)
		{
			const std::string & name = eve->GetName();
			SharedTcpSession session = this->Create(name, address);
			if (session == nullptr || !session->StartConnect())
			{
				Net2MainEvent * eve = new Net2MainEvent(SocketConnectFail, address, name);
				this->mNetProxyManager->AddNetSessionEvent(eve);
			}
		}
		else if (eve->GetEventType() == SocketSendMsgEvent)
		{
			SharedTcpSession session = this->GetSession(address);
			PB::NetWorkPacket * messageData = eve->GetMsgData();
			char * bufferStartPos = this->mSendSharedBuffer + sizeof(unsigned int);
			if (!messageData->SerializeToArray(bufferStartPos, ASIO_TCP_SEND_MAX_COUNT))
			{
				NetPacketPool.Destory(messageData);
				SayNoDebugError("Serialize Fail : " << messageData->method());
			}
			else
			{
				size_t size = messageData->ByteSizeLong();
				size_t length = size + sizeof(unsigned int);
				memcpy(this->mSendSharedBuffer, &size, sizeof(unsigned int));
				if (session->SendPackage(make_shared<string>(this->mSendSharedBuffer, length)))
				{
					NetPacketPool.Destory(messageData);				
				}
				else
				{
					const std::string & name = session->GetSessionName();
					Net2MainEvent * eve = new Net2MainEvent(Net2MainEventType::SocketSendMsgFail, address, name, messageData);
					this->mNetProxyManager->AddNetSessionEvent(eve);
				}			
			}		
		}
	}
	shared_ptr<TcpClientSession> NetSessionManager::GetSession(const std::string & address)
	{
#ifdef _DEBUG
		SayNoAssertRetNull_F(this->IsInNetThead());
#endif
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			return iter->second->IsActive() ? iter->second : nullptr;
		}
		return nullptr;
	}
	bool NetSessionManager::DescorySession(const std::string & address)
	{
		auto iter = this->mSessionAdressMap.find(address);
		if (iter != this->mSessionAdressMap.end())
		{
			SharedTcpSession session = iter->second;
			if (session->IsActive())
			{
				session->StartClose();
			}
			this->mSessionAdressMap.erase(iter);
			SayNoDebugError("remove tcp session : " << address);
			return true;
		}
		return false;
	}
}