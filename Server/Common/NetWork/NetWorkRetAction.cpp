#include"NetWorkRetAction.h"
#include<Util/TimeHelper.h>
#include<Util/ProtocHelper.h>
#include<Pool/ProtocolPool.h>
#include<Coroutine/CoroutineManager.h>
namespace Sentry
{

	LocalLuaRetActionProxy::LocalLuaRetActionProxy(NetLuaRetAction * action) : mBindLuaAction(action)
	{
		
	}

	void LocalLuaRetActionProxy::Invoke(com::NetWorkPacket * backData)
	{
		XCode code = (XCode)backData->code();
		const std::string & name = backData->protocname();
		const std::string & message = backData->messagedata();
		if (!message.empty())
		{
			Message * pMessage = GprotocolPool.Create(name);
			if (pMessage != nullptr && pMessage->ParseFromString(message))
			{
				this->mBindLuaAction->Inovke(code, pMessage);
				GprotocolPool.Destory(pMessage);
				return;
			}
			this->mBindLuaAction->Inovke(code, message);
			GprotocolPool.Destory(pMessage);
			return;
		}
		this->mBindLuaAction->Inovke(code);
	}

	LocalRetActionProxy::LocalRetActionProxy()
	{		
		this->mCreateTime = TimeHelper::GetMilTimestamp();
	}

	void LocalRetActionProxy1::Invoke(com::NetWorkPacket * backData)
	{
		this->mBindAction((XCode)backData->code());
	}
	LocalWaitRetActionProxy::LocalWaitRetActionProxy(NetLuaWaitAction * action) : mBindLuaAction(action)
	{

	}

	void LocalWaitRetActionProxy::Invoke(com::NetWorkPacket * backData)
	{
		XCode code = (XCode)backData->code();
		const std::string & name = backData->protocname();
		const std::string & message = backData->messagedata();
		if (!message.empty())
		{
			Message * pMessage = GprotocolPool.Create(name);
			if (pMessage != nullptr && pMessage->ParseFromString(message))
			{
				this->mBindLuaAction->Inovke(code, pMessage);
				GprotocolPool.Destory(pMessage);
				return;
			}
			this->mBindLuaAction->Inovke(code, message);
			GprotocolPool.Destory(pMessage);
			return;
		}
		this->mBindLuaAction->Inovke(code);
	}
	NetWorkWaitCorAction::NetWorkWaitCorAction(CoroutineManager * mgr)
	{
		this->mScheduler = mgr;
		this->mCoroutineId = mgr->GetCurrentCorId();
	}

	shared_ptr<NetWorkWaitCorAction> NetWorkWaitCorAction::Create(CoroutineManager * coroutineMgr)
	{
		if (coroutineMgr->IsInMainCoroutine())
		{
			return nullptr;
		}
		return std::make_shared<NetWorkWaitCorAction>(coroutineMgr);
	}

	void NetWorkWaitCorAction::Invoke(com::NetWorkPacket * backData)
	{
		this->mCode = (XCode)backData->code();
		this->mMessage = backData->messagedata();
		this->mScheduler->Resume(mCoroutineId);
	}
}
