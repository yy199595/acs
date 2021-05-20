#include"SessionManager.h"
#include"NetWorkManager.h"
#include"ActionManager.h"
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
		this->mErrorSessionQueue.AddItem(tcpSession);
		return true;
	}

	void SessionManager::AddRecvMessage(SharedTcpSession session, const char * message, size_t size)
	{
		if (session != nullptr)
		{
			shared_ptr<NetWorkPacket> netPacket = make_shared<NetWorkPacket>();
			if (!netPacket->ParseFromArray(message, size))
			{
				session->StartClose();
				SayNoDebugError("parse message error close session");
				return;
			}
			const std::string & address = session->GetAddress();
			mRecvMessageQueue.AddItem(make_shared<NetMessageBuffer>(address, netPacket));
		}
	}

	bool SessionManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mCoroutineSheduler = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ReConnectTime", this->mReConnectTime));
		return true;
	}

	void SessionManager::OnRecvNewMessageAfter(SharedTcpSession tcpSession, SharedPacket requestData)
	{
		this->mCurrentSession = tcpSession;
		const std::string & serviceName = requestData->service();
		const std::string & actionName = requestData->action();
		if (!serviceName.empty() && !actionName.empty())
		{
			if (!this->CallLuaAction(requestData))
			{
				this->CallAction(requestData);
			}
		}
		else if (requestData->callback_id() != 0)
		{
			const long long callbackId = requestData->callback_id();
			auto callback = this->mActionManager->GetCallback(callbackId);
			if (callback == nullptr)
			{
				SayNoDebugError("not find call back " << callbackId);
				return;
			}
			callback->Invoke(requestData);
		}
		this->mCurrentSession = nullptr;
	}

	bool SessionManager::CallAction(SharedPacket requestData)
	{
		const std::string & serviceName = requestData->service();
		const std::string & actionName = requestData->action();
		ServiceBase * service = this->GetApp()->GetService(serviceName);
		if (service == nullptr)
		{
			SayNoDebugError("not find service " << serviceName);
			return false;
		}
		const std::string name = serviceName + "." + actionName;
		const std::string address = this->mCurrentSession->GetAddress();
		this->mCoroutineSheduler->Start(name, [address, this, service, requestData]()
		{
			long long callbackId = requestData->callback_id();
			SharedPacket returnData = make_shared<NetWorkPacket>();
			XCode code = service->CallAction(requestData, returnData);
			if (callbackId != 0)
			{
				returnData->set_callback_id(callbackId);
				returnData->set_operator_id(requestData->operator_id());
				this->mNetWorkManager->SendMessageByAdress(address, returnData);
			}
		});
		return true;
	}

	bool SessionManager::CallLuaAction(SharedPacket requestData)
	{
		//const std::string & serviceName = requestData->service();
		//const std::string & actionName = requestData->action();
		//const std::string & address = this->mCurrentSession->GetAddress();
		//shared_ptr<NetLuaAction> luaAction = this->mActionManager->GetLuaAction(name);
		//if (luaAction == nullptr)	//lua 函数自己返回
		//{
		//	return XCode::CallFunctionNotExist;
		//}
		//	
		//	XCode code = luaAction->Invoke(address, requestData);
		//	const long long callbackId = requestData->callback_id();
		//	if (code != XCode::LuaCoroutineReturn && callbackId != 0)
		//	{
		//		shared_ptr<NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();
		//		returnPacket->set_error_code(code);
		//		returnPacket->set_callback_id(callbackId);
		//		returnPacket->set_operator_id(packet->operator_id());
		//		this->mNetWorkManager->SendMessageByAdress(address, returnPacket);
		//	}
		//}
		return false;
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