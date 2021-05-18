
#include"RemoteScheduler.h"
#include<Core/Applocation.h>
#include<Util/ProtocHelper.h>
#include<Script/LuaParameter.h>
#include<Other/ObjectFactory.h>
#include<Manager/LocalActionManager.h>
namespace SoEasy
{
	RemoteScheduler::RemoteScheduler(long long operaotrId)
	{
		this->mOperatorId = operaotrId;
		Applocation * pApplocation = Applocation::Get();
		this->mNetWorkManager = pApplocation->GetManager<NetWorkManager>();
		this->mFunctionManager = pApplocation->GetManager<LocalActionManager>();
	}

	RemoteScheduler::RemoteScheduler(shared_ptr<TcpClientSession> session, long long operId)
	{
		this->mOperatorId = operId;
		this->mBindSessionAdress = session->GetAddress();
		Applocation * pApplocation = Applocation::Get();
		this->mNetWorkManager = pApplocation->GetManager<NetWorkManager>();
		this->mFunctionManager = pApplocation->GetManager<LocalActionManager>();
	}

	XCode RemoteScheduler::Call(std::string func)
	{
		return this->SendCallMessage(func);
	}

	XCode RemoteScheduler::Call(std::string func, LuaTable & luaTable)
	{
		return this->SendCallMessage(func, luaTable);
	}

	XCode RemoteScheduler::Call(std::string func, Message * message)
	{
		return this->SendCallMessage(func, message);
	}
	XCode RemoteScheduler::Call(std::string func, Message * message, NetWorkRetAction1 action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalRetActionProxy1>(action, func);
		return this->SendCallMessage(func, message, pAction);
	}

	XCode RemoteScheduler::Call(std::string func, NetWorkRetAction1 action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalRetActionProxy1>(action, func);
		return this->SendCallMessage(func, nullptr, pAction);
	}
}

namespace SoEasy
{
	XCode RemoteScheduler::Call(std::string func, Message * message, NetLuaRetAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalLuaRetActionProxy>(action, func);
		return this->SendCallMessage(func, message, pAction);
	}

	XCode RemoteScheduler::Call(std::string func, LuaTable & luaTable, NetLuaRetAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalLuaRetActionProxy>(action, func);
		return this->SendCallMessage(func, luaTable, pAction);
	}

	XCode RemoteScheduler::Call(std::string func, LuaTable & luaTable, NetLuaWaitAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalWaitRetActionProxy>(action, func);
		return this->SendCallMessage(func, luaTable, pAction);
	}

	XCode RemoteScheduler::SendCallMessage(std::string & func, Message * message, shared_ptr<LocalRetActionProxy> action)
	{
		mMessageBuffer.clear();
		shared_ptr<NetWorkPacket> newCallData = make_shared<NetWorkPacket>();
		if (message != nullptr && message->SerializePartialToString(&mMessageBuffer))
		{
			newCallData->set_message_data(mMessageBuffer);
			newCallData->set_protoc_name(message->GetTypeName());
		}
		long long callbackId = 0;
		this->mFunctionManager->AddCallback(action, callbackId);
		newCallData->set_func_name(func);
		newCallData->set_callback_id(callbackId);
		newCallData->set_operator_id(mOperatorId);
		if (this->mBindSessionAdress.empty())
		{
			return this->mNetWorkManager->SendMessageByName(func, newCallData);
		}
		return this->mNetWorkManager->SendMessageByAdress(this->mBindSessionAdress, newCallData);
	}

	XCode RemoteScheduler::SendCallMessage(std::string & func, LuaTable & luaTable, shared_ptr<LocalRetActionProxy> action)
	{
		mMessageBuffer.clear();
		shared_ptr<NetWorkPacket> newCallData = make_shared<NetWorkPacket>();
		if (!luaTable.Serialization(mMessageBuffer))
		{
			return XCode::SerializationFailure;
		}
		long long callbakcId = 0;
		newCallData->set_message_data(mMessageBuffer);
		this->mFunctionManager->AddCallback(action, callbakcId);
		newCallData->set_func_name(func);
		newCallData->set_callback_id(callbakcId);
		newCallData->set_operator_id(mOperatorId);
		return this->mNetWorkManager->SendMessageByAdress(this->mBindSessionAdress, newCallData);
	}

	XCode RemoteScheduler::Call(std::string func, Message * message, NetLuaWaitAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalWaitRetActionProxy>(action, func);
		return this->SendCallMessage(func, message, pAction);
	}
}