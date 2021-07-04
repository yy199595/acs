#include"NetProxyManager.h"
#include"NetSessionManager.h"
#include"ServiceManager.h"
#include<Core/Applocation.h>
#include<Manager/TimerManager.h>
#include<Manager/ActionManager.h>
namespace SoEasy
{
	
	bool NetProxyManager::AddNetSessionEvent(Net2MainEvent * eve)
	{
		if (eve == nullptr) return false;
		this->mNetEventQueue.AddItem(eve);
		return false;
	}

	bool NetProxyManager::DescorySession(const std::string & address)
	{
		Main2NetEvent * eve = new Main2NetEvent(SocketDectoryEvent, address);
		return this->mNetWorkManager->AddNetSessionEvent(eve);
	}

	bool NetProxyManager::SendMsgByAddress(const std::string & address, PB::NetWorkPacket * msgData)
	{
		if (msgData == nullptr)
		{
			return false;
		}
		auto iter = this->mSessionMap.find(address);
		if (iter == this->mSessionMap.end())
		{
			return false;
		}
		Main2NetEvent * eve = new Main2NetEvent(SocketSendMsgEvent, address, "", msgData);
		return this->mNetWorkManager->AddNetSessionEvent(eve);
	}

	bool NetProxyManager::ConnectByAddress(const std::string & address, const std::string & name, int delayTime)
	{
		auto iter = this->mConnectSessions.find(address);
		if (iter != this->mConnectSessions.end())
		{
			return true;
		}
		if (delayTime == 0)
		{
			this->mConnectSessions.insert(address);
			Main2NetEvent * eve = new Main2NetEvent(SocketConnectEvent, address, name);
			return this->mNetWorkManager->AddNetSessionEvent(eve);
		}
		return this->mTimerManager->AddTimer(delayTime, [this, address, name]()
			{
				this->mConnectSessions.insert(address);
				Main2NetEvent * eve = new Main2NetEvent(SocketConnectEvent, address, name);			
				return this->mNetWorkManager->AddNetSessionEvent(eve);
			});
	}

	bool NetProxyManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mTimerManager = this->GetManager<TimerManager>());
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetSessionManager>());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ReConnectTime", this->mReConnectTime));
		this->mReConnectTime = this->mReConnectTime * 1000;
		return true;
	}

	void NetProxyManager::OnSystemUpdate()
	{
		shared_ptr<TcpClientSession> pTcpSession = nullptr;

		Net2MainEvent * eve = nullptr;
		this->mNetEventQueue.SwapQueueData();
		while (this->mNetEventQueue.PopItem(eve))
		{
			this->HandlerNetEvent(eve);
			const std::string & address = eve->GetAddress();
			if (eve->GetEventType() == Net2MainEventType::SocketConnectSuc)
			{
				this->RemoveSessionByAddress(address);			
				auto iter = this->mConnectSessions.find(address);
				if (iter != this->mConnectSessions.end())
				{
					this->mConnectSessions.erase(iter);
				}
				this->mSessionMap.emplace(address, SessionNodeType);
				SayNoDebugInfo("connect to " << address << " successful");
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketNewConnect)
			{
				this->RemoveSessionByAddress(address);
				this->mSessionMap.emplace(address, SessionClientType);
				SayNoDebugInfo("new socket connect : " << address);

			}
			else if (eve->GetEventType() == Net2MainEventType::SocketConnectFail)
			{
				this->RemoveSessionByAddress(address);
				const std::string & name = eve->GetName();
				SayNoDebugWarning("connect " << name << " " << address << "fail");
				this->ConnectByAddress(address, name, this->mReConnectTime);
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketReceiveFail)
			{
				auto iter = this->mSessionMap.find(address);
				if (iter != this->mSessionMap.end())
				{
					if (iter->second == SessionType::SessionNodeType)
					{
						const std::string & name = eve->GetName();
						this->ConnectByAddress(address, name, this->mReConnectTime);
					}
					else
					{
						Main2NetEvent * e = new Main2NetEvent(Main2NetEventType::SocketDectoryEvent, address);
						this->mNetWorkManager->AddNetSessionEvent(e);
					}
					this->mSessionMap.erase(iter);
				}
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketSendMsgFail)
			{
				const std::string & name = eve->GetName();
				PB::NetWorkPacket * msgData = eve->GetMsgData();
				SayNoDebugError("send " << msgData->service() << "." << msgData->method() << " error");
				NetPacketPool.Destory(msgData);
			}
			else if (eve->GetEventType() == Net2MainEventType::SocketReceiveData)
			{
				PB::NetWorkPacket * msgData = eve->GetMsgData();
				if (!this->OnRecvMessage(address, msgData))
				{
					this->RemoveSessionByAddress(address);
					Main2NetEvent * e = new Main2NetEvent(SocketDectoryEvent, address);
					this->mNetWorkManager->AddNetSessionEvent(e);
				}
			}
			delete eve;
		}
	}

	bool NetProxyManager::OnRecvMessage(const std::string & address, PB::NetWorkPacket * messageData)
	{
		const std::string & method = messageData->method();
		const std::string & service = messageData->service();
		const long long callbackId = messageData->rpcid();
		if (!service.empty() && !method.empty())
		{
			if (!mServiceManager->HandlerMessage(address, messageData))
			{
				return false;
			}
		}
		long long rpcId = messageData->rpcid();
		return this->mActionManager->InvokeCallback(rpcId, messageData);
	}
	bool NetProxyManager::HandlerNetEvent(Net2MainEvent * eve)
	{
		return false;
	}
	bool NetProxyManager::RemoveSessionByAddress(const std::string & address)
	{
		auto iter = this->mSessionMap.find(address);
		if (iter != this->mSessionMap.end())
		{
			this->mSessionMap.erase(iter);
			return true;
		}
		return false;
	}
}