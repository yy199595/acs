#include"ServiceBase.h"
#include<Manager/ActionManager.h>
#include<Manager/NetWorkManager.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	ServiceBase::ServiceBase()
	{
		this->mIsInit = false;
	}
	bool ServiceBase::OnInit()
	{
		SayNoAssertRetFalse_F(this->mNetManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
		return true;
	}

	void ServiceBase::OnSystemUpdate()
	{
		SharedPacket localPacket;
		SharedNetPacket proxyService;
		this->mLocalHandleMsgQueue.SwapQueueData();
		this->mProxyHandleMsgQueue.SwapQueueData();
		while (this->mLocalHandleMsgQueue.PopItem(localPacket))
		{
			const std::string & method = localPacket->method();
			if (!this->InvokeMethod(method, localPacket))
			{
				SayNoDebugError("call " << this->GetServiceName() << "." << method << " fail");
			}
		}
		while (this->mProxyHandleMsgQueue.PopItem(proxyService))
		{
			const std::string & address = proxyService->mAddress;
			const std::string & method = proxyService->mMessagePacket->method();
			if (!this->InvokeMethod(address, method, proxyService->mMessagePacket))
			{
				SayNoDebugError(address << " call " << this->GetServiceName() << "." << method << " fail");
			}
		}
	}

	void ServiceBase::PushHandleMessage(SharedPacket argv)
	{
		this->mLocalHandleMsgQueue.AddItem(argv);
	}

	void ServiceBase::PushHandleMessage(const std::string & address, SharedPacket argv)
	{
		SharedNetPacket packet = make_shared<NetMessageBuffer>(address, argv);
		this->mProxyHandleMsgQueue.AddItem(packet);
	}

	bool ServiceBase::ReplyMessage(const std::string & address, shared_ptr<NetWorkPacket> msg)
	{
		SayNoAssertRetFalse_F(this->mNetManager);
		return this->mNetManager->SendMessageByAdress(address, msg);
	}

	bool ServiceBase::ReplyMessage(const long long cb, shared_ptr<NetWorkPacket> msg)
	{
		SayNoAssertRetFalse_F(this->mActionManager);
		return this->mActionManager->AddActionArgv(cb, msg);
	}

	void ServiceBase::Sleep(long long ms)
	{
		SayNoAssertRet_F(this->mCorManager->IsInMainCoroutine());
		this->mCorManager->Sleep(ms);
	}

	void ServiceBase::Start(const std::string & name, std::function<void()> && func)
	{
		SayNoAssertRet_F(this->mCorManager->IsInMainCoroutine());
		this->mCorManager->Start(name, std::move(func));
	}

	void ServiceBase::InitService(const std::string & serviceName)
	{
		if (this->mIsInit == false)
		{
			this->mIsInit = true;
			this->mServiceName = serviceName;
			this->Init(Applocation::Get(), serviceName);
		}
	}

	XCode ServiceBase::Call(const std::string method, Message & returnData)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(method, this->mCorManager);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		this->PushHandleMessage(this->CreatePacket(method, nullptr, callBack));
		this->mCorManager->YieldReturn();
		if (!returnData.ParseFromString(callBack->GetMsgData()))
		{
			return XCode::ParseMessageError;
		}
		return callBack->GetCode();
	}

	XCode ServiceBase::Call(const std::string method, shared_ptr<Message> message)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(method, this->mCorManager);
		if (this->mCorManager->IsInMainCoroutine())
		{
			return XCode::NoCoroutineContext;
		}
		this->PushHandleMessage(this->CreatePacket(method, message, callBack));
		this->mCorManager->YieldReturn();
		return callBack->GetCode();
	}

	XCode ServiceBase::Call(const std::string method, shared_ptr<Message> message, Message & returnData)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(method, this->mCorManager);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		this->PushHandleMessage(this->CreatePacket(method, message, callBack));
		this->mCorManager->YieldReturn();
		if (!returnData.ParseFromString(callBack->GetMsgData()))
		{
			return XCode::ParseMessageError;
		}
		return callBack->GetCode();
	}

	XCode ServiceBase::Notice(const std::string method, shared_ptr<Message> message)
	{
		this->PushHandleMessage(this->CreatePacket(method, message, nullptr));
		return XCode::Failure;
	}

	shared_ptr<NetWorkPacket> ServiceBase::CreatePacket(const std::string & func, shared_ptr<Message> message, shared_ptr<NetWorkWaitCorAction> callBack)
	{
		shared_ptr<NetWorkPacket> callData = make_shared<NetWorkPacket>();
		if (message != nullptr)
		{
			std::string messageBuffer;
			if (!message->SerializePartialToString(&messageBuffer))
			{
				SayNoDebugError(func << " " << message->GetTypeName() << " Serialize fail");
				return nullptr;
			}
			callData->set_messagedata(messageBuffer);
			callData->set_protocname(message->GetTypeName());
		}
		if (callBack != nullptr)
		{
			callData->set_rpcid(this->mActionManager->AddCallback(callBack));
		}		
		callData->set_method(func);
		callData->set_service(this->GetTypeName());
		return callData;
	}
}
