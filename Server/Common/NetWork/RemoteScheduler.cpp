
#include"RemoteScheduler.h"
#include<Core/Applocation.h>
#include<Util/ProtocHelper.h>
#include<Script/LuaParameter.h>
#include<Other/ObjectFactory.h>
#include<Manager/ActionManager.h>
namespace SoEasy
{
	RemoteScheduler::RemoteScheduler(const std::string & address)
	{
		this->mBindSessionAdress = address;
		Applocation * pApplocation = Applocation::Get();
		this->mNetWorkManager = pApplocation->GetManager<NetWorkManager>();
		this->mFunctionManager = pApplocation->GetManager<ActionManager>();
	}
	RemoteScheduler::RemoteScheduler(shared_ptr<TcpClientSession> session)
	{
		Applocation * app = Applocation::Get();
		this->mBindSessionAdress = session->GetAddress();
		this->mNetWorkManager = app->GetManager<NetWorkManager>();
		this->mFunctionManager = app->GetManager<ActionManager>();
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func)
	{
		return this->SendCallMessage(service, func);
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func, LuaTable & luaTable)
	{
		return this->SendCallMessage(service, func, luaTable);
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func, Message * message)
	{
		return this->SendCallMessage(service, func, message);
	}
	XCode RemoteScheduler::Call(const std::string service, std::string func, Message * message, NetWorkRetAction1 action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalRetActionProxy1>(action, func);
		return this->SendCallMessage(service, func, message, pAction);
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func, NetWorkRetAction1 action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalRetActionProxy1>(action, func);
		return this->SendCallMessage(service, func, nullptr, pAction);
	}
}

namespace SoEasy
{
	XCode RemoteScheduler::Call(const std::string service, std::string func, Message * message, NetLuaRetAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalLuaRetActionProxy>(action, func);
		return this->SendCallMessage(service, func, message, pAction);
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func, LuaTable & luaTable, NetLuaRetAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalLuaRetActionProxy>(action, func);
		return this->SendCallMessage(service, func, luaTable, pAction);
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func, LuaTable & luaTable, NetLuaWaitAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalWaitRetActionProxy>(action, func);
		return this->SendCallMessage(service, func, luaTable, pAction);
	}

	XCode RemoteScheduler::SendCallMessage(const std::string & service, std::string & func, Message * message, shared_ptr<LocalRetActionProxy> action)
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
		newCallData->set_service(service);
		newCallData->set_action(func);
		newCallData->set_callback_id(callbackId);
		return this->mNetWorkManager->SendMessageByAdress(this->mBindSessionAdress, newCallData);
	}

	XCode RemoteScheduler::SendCallMessage(const std::string & service, std::string & func, LuaTable & luaTable, shared_ptr<LocalRetActionProxy> action)
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

		newCallData->set_service(service);
		newCallData->set_service(func);

		newCallData->set_callback_id(callbakcId);
		return this->mNetWorkManager->SendMessageByAdress(this->mBindSessionAdress, newCallData);
	}

	XCode RemoteScheduler::Call(const std::string service, std::string func, Message * message, NetLuaWaitAction * action)
	{
		shared_ptr<LocalRetActionProxy> pAction = make_shared<LocalWaitRetActionProxy>(action, func);
		return this->SendCallMessage(service, func, message, pAction);
	}
}