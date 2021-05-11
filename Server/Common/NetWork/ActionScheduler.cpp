#include "ActionScheduler.h"
#include<Manager/NetWorkManager.h>
#include<Manager/LocalActionManager.h>
#include<Coroutine/CoroutineManager.h>
#include<NetWork/NetWorkRetAction.h>
namespace SoEasy
{
	ActionScheduler::ActionScheduler(long long operId)
	{
		this->InitScheduler();
		this->mOperatorId = operId;
	}

	ActionScheduler::ActionScheduler(shared_ptr<TcpClientSession> session)
	{
		this->InitScheduler();
		this->mOperatorId = session->GetSocketId();
		this->mSessionAddress = session->GetAddress();
	}

	void ActionScheduler::InitScheduler()
	{
		Applocation * app = Applocation::Get();
		this->mNetWorkManager = app->GetManager<NetWorkManager>();
		this->mActionManager = app->GetManager<LocalActionManager>();
		this->mCoroutineScheduler = app->GetManager<CoroutineManager>();
	}

	XCode ActionScheduler::Call(const std::string func, Message & returnData)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(func, this->mCoroutineScheduler);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		XCode code = this->SendCallMessage(func, nullptr, callBack);
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

	XCode ActionScheduler::Call(const std::string func, const Message * message)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(func, this->mCoroutineScheduler);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		XCode code = this->SendCallMessage(func, message, callBack);
		if (code == XCode::Successful)
		{
			mCoroutineScheduler->YieldReturn();
			return callBack->GetCode();
		}
		return XCode::Failure;
	}

	XCode ActionScheduler::Call(const std::string func, const Message * message, Message & returnData)
	{
		shared_ptr<NetWorkWaitCorAction> callBack = NetWorkWaitCorAction::Create(func, this->mCoroutineScheduler);
		if (callBack == nullptr)
		{
			return XCode::NoCoroutineContext;
		}
		XCode code = this->SendCallMessage(func, message, callBack);
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

	

	XCode ActionScheduler::SendCallMessage(const std::string & func, const Message * message, shared_ptr<LocalRetActionProxy> callBack)
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
		callData->set_func_name(func);
		callData->set_callback_id(id);
		if (this->mSessionAddress.empty())
		{
			if (!this->mNetWorkManager->SendMessageByName(func, callData))
			{
				return XCode::SendMessageFail;
			}
		}	
		return this->mNetWorkManager->SendMessageByAdress(mSessionAddress, callData);
	}

}
