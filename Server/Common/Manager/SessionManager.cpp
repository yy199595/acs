#include"SessionManager.h"
#include"NetWorkManager.h"
#include"ServiceManager.h"
#include<Core/Applocation.h>
#include<Util/StringHelper.h>
#include<NetWork/NetWorkRetAction.h>
#include<Service/LocalService.h>
#include<Service/LocalLuaService.h>
#include<Coroutine/CoroutineManager.h>
#include<Manager/ActionManager.h>
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
		this->mNewSessionQueue.AddItem(tcpSession);
		return true;
	}

	bool SessionManager::AddErrorSession(SharedTcpSession tcpSession)
	{
		if (!tcpSession)
		{
			return false;
		}
		this->mErrorSessionQueue.AddItem(tcpSession);
		return true;
	}

	bool SessionManager::AddRecvMessage(SharedTcpSession session, const char * message, size_t size)
	{
		shared_ptr<NetWorkPacket> messageData = make_shared<NetWorkPacket>();
		if (!messageData->ParseFromArray(message, size))
		{
			session->StartClose();
			SayNoDebugError("parse message error close session");
			return false;
		}
		const std::string & method = messageData->method();
		const std::string & service = messageData->service();
		const long long callbackId = messageData->rpcid();
		if (!service.empty() && !method.empty())
		{
			const std::string & address = session->GetAddress();
			LocalLuaService * luaService = this->mServiceManager->GetLuaService(service);
			if (luaService != nullptr && luaService->HasMethod(method))
			{
				luaService->PushHandleMessage(address, messageData);
				SayNoDebugInfo("call lua method " << service << "." << method);
				return true;
			}
			LocalService * localService = this->mServiceManager->GetLocalService(service);
			if (localService != nullptr && localService->HasMethod(method))
			{
				localService->PushHandleMessage(address, messageData);
				SayNoDebugInfo("call c++ method " << service << "." << method);
				return true;
			}
			return false;
		}
		this->mActionManager->AddActionArgv(callbackId, messageData);
		return true;
	}

	bool SessionManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mCoroutineSheduler = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ReConnectTime", this->mReConnectTime));
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
			if (pTcpSession->IsContent() && pTcpSession->GetState() != Session_Close) //断线重连(代码关闭不重连)
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