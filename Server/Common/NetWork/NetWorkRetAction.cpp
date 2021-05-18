#include"NetWorkRetAction.h"
#include<Util/TimeHelper.h>
#include<Util/ProtocHelper.h>
#include<Other/ObjectFactory.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{

	LocalLuaRetActionProxy::LocalLuaRetActionProxy(NetLuaRetAction * action, std::string name)
		: LocalRetActionProxy(name), mBindLuaAction(action)
	{
		
	}

	void LocalLuaRetActionProxy::Invoke(const shared_ptr<NetWorkPacket> backData)
	{
		XCode code = (XCode)backData->error_code();
		const std::string & name = backData->protoc_name();
		const std::string & message = backData->message_data();
		if (!message.empty())
		{
			Message * pMessage = ObjectFactory::Get()->CreateMessage(name);
			if (pMessage != nullptr && pMessage->ParseFromString(message))
			{
				this->mBindLuaAction->Inovke(code, pMessage);
				return;
			}
			this->mBindLuaAction->Inovke(code, message);
			return;
		}
		this->mBindLuaAction->Inovke(code);
	}

	LocalRetActionProxy::LocalRetActionProxy(const std::string & name)
	{
		this->mFunctionName = name;
		this->mCreateTime = TimeHelper::GetMilTimestamp();
	}
	void LocalRetActionProxy1::Invoke(const shared_ptr<NetWorkPacket> backData)
	{
		this->mBindAction((XCode)backData->error_code());
	}
	LocalWaitRetActionProxy::LocalWaitRetActionProxy(NetLuaWaitAction * action, std::string name)
		:LocalRetActionProxy(name), mBindLuaAction(action)
	{

	}

	void LocalWaitRetActionProxy::Invoke(const shared_ptr<NetWorkPacket> backData)
	{
		XCode code = (XCode)backData->error_code();
		const std::string & name = backData->protoc_name();
		const std::string & message = backData->message_data();
		if (!message.empty())
		{
			Message * pMessage = ObjectFactory::Get()->CreateMessage(name);
			if (pMessage != nullptr && pMessage->ParseFromString(message))
			{
				this->mBindLuaAction->Inovke(code, pMessage);
				return;
			}
			this->mBindLuaAction->Inovke(code, message);
			return;
		}
		this->mBindLuaAction->Inovke(code);
	}
	NetWorkWaitCorAction::NetWorkWaitCorAction(std::string name, CoroutineManager * mgr)
		: LocalRetActionProxy(name)
	{
		this->mScheduler = mgr;
		this->mCoroutineId = mgr->GetCurrentCorId();
	}

	shared_ptr<NetWorkWaitCorAction> NetWorkWaitCorAction::Create(std::string name, CoroutineManager * coroutineMgr)
	{
		if (coroutineMgr->GetCoroutine() != nullptr)
		{
			return std::make_shared<NetWorkWaitCorAction>(name, coroutineMgr);
		}
		return nullptr;
	}

	void NetWorkWaitCorAction::Invoke(const shared_ptr<NetWorkPacket> backData)
	{
		this->mCode = (XCode)backData->error_code();
		this->mMessage = backData->message_data();
		this->mScheduler->Resume(mCoroutineId);
	}
}
