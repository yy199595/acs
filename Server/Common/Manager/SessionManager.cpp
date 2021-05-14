#include"SessionManager.h"
#include"NetWorkManager.h"
#include"LocalActionManager.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<NetWork/NetLuaAction.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	SharedTcpSession SessionManager::CreateTcpSession(SharedTcpSocket socket)
	{
		shared_ptr<TcpClientSession> tcpSession = std::make_shared<TcpClientSession>(this, socket);
		if (tcpSession != nullptr)
		{
			this->mNewSessionQueue.AddItem(tcpSession);
			return tcpSession;
		}
		return nullptr;
	}

	SharedTcpSession SessionManager::CreateTcpSession(std::string name, std::string address)
	{
		std::string connectIp;
		unsigned short connectPort;
		if (!StringHelper::ParseIpAddress(address, connectIp, connectPort))
		{
			SayNoDebugError("parse " << address << " fail")
			return nullptr;
		}
		return this->CreateTcpSession(name, connectIp, connectPort);
	}

	SharedTcpSession SessionManager::CreateTcpSession(std::string name, std::string ip, unsigned short port)
	{
		SharedTcpSession tcpSession = std::make_shared<TcpClientSession>(this, name, ip, port);
		if (tcpSession != nullptr)
		{
			tcpSession->StartConnect();
		}
		return tcpSession;
	}

	bool SessionManager::AddNewSession(SharedTcpSession tcpSession)
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

	bool SessionManager::AddErrorSession(SharedTcpSession tcpSession)
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

	void SessionManager::AddRecvMessage(SharedTcpSession session, const char * message, size_t size)
	{
		if (session != nullptr)
		{
			shared_ptr<NetWorkPacket> netPacket = make_shared<NetWorkPacket>();
			if (netPacket->ParseFromArray(message, size))
			{
				const std::string & address = session->GetAddress();
				mRecvMessageQueue.AddItem(make_shared<NetMessageBuffer>(address, netPacket));
			}
		}
	}

	bool SessionManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<LocalActionManager>());
		SayNoAssertRetFalse_F(this->mCoroutineSheduler = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ReConnectTime", this->mReConnectTime));
		return true;
	}

	void SessionManager::OnRecvNewMessageAfter(SharedTcpSession tcpSession, shared_ptr<NetWorkPacket> packet)
	{
		if (this->mRecvMsgCallback != nullptr)
		{
			if (this->mRecvMsgCallback(tcpSession, packet))
			{
				return;
			}
		}
		this->mCurrentSession = tcpSession;
		const std::string & address = tcpSession->GetAddress();
		if (!packet->func_name().empty())
		{
			const std::string & name = packet->func_name();
	
			shared_ptr<NetLuaAction> luaAction = this->mActionManager->GetLuaAction(name);
			if (luaAction != nullptr)	//lua 函数自己返回
			{			
				XCode code = luaAction->Invoke(address, packet);
				if (code != XCode::LuaCoroutineReturn)
				{
					shared_ptr<NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();
					returnPacket->set_error_code(code);
					returnPacket->set_operator_id(packet->operator_id());
					returnPacket->set_callback_id(packet->callback_id());
					this->mNetWorkManager->SendMessageByAdress(address, returnPacket);
				}
			}
			else  //在协程中执行
			{
				this->mCoroutineSheduler->Start([this, packet, address]()
				{
					shared_ptr<NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();
					XCode code = this->InvokeAction(this->mCurrentSession, packet, returnPacket);
					if (packet->callback_id() != 0)
					{
						returnPacket->set_error_code(code);
						returnPacket->set_operator_id(packet->operator_id());
						returnPacket->set_callback_id(packet->callback_id());
						this->mNetWorkManager->SendMessageByAdress(address, returnPacket);
					}
				});
			}
		}
		else if (packet->callback_id() != 0)
		{
			const long long id = packet->callback_id();
			auto callback = this->mActionManager->GetCallback(id);
			if (callback == nullptr)
			{
				SayNoDebugError("not find call back " << id);
				return;
			}
			callback->Invoke(this->mCurrentSession, packet);
		}
		this->mCurrentSession = nullptr;
	}

	XCode SessionManager::InvokeAction(shared_ptr<TcpClientSession> tcpSession, shared_ptr<NetWorkPacket> callInfo, shared_ptr<NetWorkPacket> returnData)
	{
		const std::string & name = callInfo->func_name();
		shared_ptr<LocalActionProxy> localActionProxy = this->mActionManager->GetAction(name);
		if (localActionProxy == nullptr)
		{
			SayNoDebugError(name << " does not exist");
			return XCode::CallFunctionNotExist;
		}	
		return localActionProxy->Invoke(callInfo, returnData);
	}

	long long SessionManager::GetIdByAddress(const std::string & address)
	{
		return 0;
	}

	bool SessionManager::ParseAddress(const std::string & address, string & ip, unsigned short & port)
	{
		size_t pos = address.find(":");
		if (pos == std::string::npos)
		{
			return false;
		}
		ip = address.substr(0, pos);
		port = (unsigned short)std::stoul(address.substr(pos + 1));
		return true;
	}

	void SessionManager::OnSystemUpdate()
	{
		shared_ptr<TcpClientSession> pTcpSession = nullptr;
		this->mNewSessionQueue.SwapQueueData();
		this->mErrorSessionQueue.SwapQueueData();

		while (this->mErrorSessionQueue.PopItem(pTcpSession))
		{
			this->mNetWorkManager->RemoveTcpSession(pTcpSession);
			if (pTcpSession->IsContent()) //断线重连
			{
				const std::string & address = pTcpSession->GetAddress();
				this->mWaitConnectSessionMap.emplace(address, pTcpSession);
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
			if (pTcpSession->IsContent())
			{
				const std::string & address = pTcpSession->GetAddress();
				auto iter = this->mWaitConnectSessionMap.find(address);
				if (iter != this->mWaitConnectSessionMap.end())
				{
					this->mWaitConnectSessionMap.erase(iter);
					SayNoDebugInfo(address << " reconnect success");
				}
			}
			pTcpSession = nullptr;
		}

		SharedNetPacket pMessagePacket;
		this->mRecvMessageQueue.SwapQueueData();
		while (this->mRecvMessageQueue.PopItem(pMessagePacket))
		{
			const std::string & address = pMessagePacket->mAddress;
			shared_ptr<NetWorkPacket> netWorkPacket = pMessagePacket->mMessagePacket;
			shared_ptr<TcpClientSession> tcpSession = mNetWorkManager->GetTcpSession(address);
			if (tcpSession != nullptr)
			{
				this->OnRecvNewMessageAfter(tcpSession, netWorkPacket);
			}		
		}
	}
	void SessionManager::OnSecondUpdate()
	{
		if (!this->mWaitConnectSessionMap.empty())
		{
			long long nowTime = TimeHelper::GetMilTimestamp();
			auto iter = this->mWaitConnectSessionMap.begin();
			for (; iter != this->mWaitConnectSessionMap.end(); iter++)
			{
				SharedTcpSession tcpSession = iter->second;
				if (nowTime - tcpSession->GetStartTime() >= this->mReConnectTime * 1000)
				{
					tcpSession->StartConnect();
				}
			}
		}
	}
}