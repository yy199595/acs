#include "NetWorkRetAction.h"
#include<CommonUtil/TimeHelper.h>
#include<CommonUtil/ProtocHelper.h>
#include<CommonOther/ObjectFactory.h>
#include<CommonCoroutine/CoroutineManager.h>
namespace SoEasy
{

	NetWorkRetActionBoxLua::NetWorkRetActionBoxLua(NetLuaRetAction * action, std::string name)
		: NetWorkRetActionBox(name), mBindLuaAction(action)
	{
		
	}

	void NetWorkRetActionBoxLua::Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & backData)
	{
		XCode code = (XCode)backData.error_code();
		const std::string & name = backData.protoc_name();
		const std::string & message = backData.message_data();
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

	NetWorkRetActionBox::NetWorkRetActionBox(std::string & name)
	{
		this->mFunctionName = name;
		this->mCreateTime = TimeHelper::GetMilTimestamp();
	}
	void NetWorkRetActionBox1::Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & backData)
	{
		XCode code = (XCode)backData.error_code();
		if (this->mBindLuaAction)
		{
			this->mBindLuaAction->Inovke(code);
			return;
		}
		this->mBindAction(session, code);
	}
	NetWorkWaitActionBoxLua::NetWorkWaitActionBoxLua(NetLuaWaitAction * action, std::string name)
		:NetWorkRetActionBox(name), mBindLuaAction(action)
	{

	}

	void NetWorkWaitActionBoxLua::Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & backData)
	{
		XCode code = (XCode)backData.error_code();
		const std::string & name = backData.protoc_name();
		const std::string & message = backData.message_data();
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
		: NetWorkRetActionBox(name)
	{
		this->mScheduler = mgr;
		this->mCoroutineId = mgr->GetCurrentCorId();
	}

	void NetWorkWaitCorAction::Invoke(shared_ptr<TcpClientSession> session, const NetWorkPacket & backData)
	{
		this->mCode = (XCode)backData.error_code();
		this->mMessage = backData.message_data();
		this->mScheduler->Resume(mCoroutineId);
	}
}
