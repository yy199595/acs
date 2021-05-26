#include "ActionScheduler.h"
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<Coroutine/CoroutineManager.h>
#include<NetWork/NetWorkRetAction.h>
namespace SoEasy
{
	ActionScheduler::ActionScheduler(const std::string & address)
	{
		this->InitScheduler();
		this->mSessionAddress = address;
	}
	ActionScheduler::ActionScheduler(shared_ptr<TcpClientSession> session)
	{
		this->InitScheduler();
		this->mSessionAddress = session->GetAddress();
	}

	void ActionScheduler::InitScheduler()
	{
		Applocation * app = Applocation::Get();
		this->mNetWorkManager = app->GetManager<NetWorkManager>();
		this->mActionManager = app->GetManager<ActionManager>();
		this->mCoroutineScheduler = app->GetManager<CoroutineManager>();
	}

	XCode ActionScheduler::Call(const std::string & service, const std::string func, Message & returnData)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(func, this->mCoroutineScheduler);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		XCode code = this->SendCallMessage(service, func, nullptr, callBack);
		if (code == XCode::Successful)
		{
			mCoroutineScheduler->YieldReturn();
			if (!returnData.ParseFromString(callBack->GetMsgData()))
			{
				return XCode::ParseMessageError;
			}
			return callBack->GetCode();
		}
		return XCode::Failure;
	}

	XCode ActionScheduler::Call(const std::string & service, const std::string func, shared_ptr<Message> message)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(func, this->mCoroutineScheduler);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		XCode code = this->SendCallMessage(service, func, message, callBack);
		if (code == XCode::Successful)
		{
			mCoroutineScheduler->YieldReturn();
			return callBack->GetCode();
		}
		return code;
	}

	XCode ActionScheduler::Call(const std::string & service, const std::string func, shared_ptr<Message> message, Message & returnData)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(func, this->mCoroutineScheduler);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		XCode code = this->SendCallMessage(service, func, message, callBack);
		if (code == XCode::Successful)
		{
			mCoroutineScheduler->YieldReturn();
			if (!returnData.ParseFromString(callBack->GetMsgData()))
			{
				return XCode::ParseMessageError;
			}
			return callBack->GetCode();
		}
		return XCode::Failure;
	}

	

	XCode ActionScheduler::SendCallMessage(const std::string & service, const std::string & func, shared_ptr<Message> message, shared_ptr<LocalRetActionProxy> callBack)
	{
		shared_ptr<NetWorkPacket> callData = make_shared<NetWorkPacket>();
		if (message != nullptr)
		{
			std::string messageBuffer;
			if (!message->SerializePartialToString(&messageBuffer))
			{
				return XCode::SerializationFailure;
			}
			callData->set_message_data(messageBuffer);
			callData->set_protoc_name(message->GetTypeName());
		}
		long long id = 0;
		this->mActionManager->AddCallback(callBack, id);
		callData->set_service(service);
		callData->set_action(func);
		callData->set_callback_id(id);
		return this->mNetWorkManager->SendMessageByAdress(mSessionAddress, callData);
	}

}
