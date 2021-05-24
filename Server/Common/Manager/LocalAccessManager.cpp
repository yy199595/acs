#include"LocalAccessManager.h"
#include"ActionManager.h"
#include<NetWork/NetLuaAction.h>
#include<NetWork/NetWorkAction.h>
#include<NetWork/NetLuaRetAction.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
#include<Service/ServiceBase.h>
namespace SoEasy
{
	bool LocalAccessManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->mLocalActionManager = this->GetManager<ActionManager>());
		return true;
	}

	void LocalAccessManager::OnSystemUpdate()
	{
		shared_ptr<NetWorkPacket> message = nullptr;
		while (!this->mLocalCallActionMessage.empty())
		{
			message = this->mLocalCallActionMessage.front();
			this->HandleLocationCallAction(message);
			this->mLocalCallActionMessage.pop();
		}
		while (!this->mLocalRetActionMessage.empty())
		{
			message = this->mLocalRetActionMessage.front();
			this->HandleLocationRetAction(message);
			this->mLocalRetActionMessage.pop();
		}
	}

	bool LocalAccessManager::CallService(const std::string & serviceName, shared_ptr<NetWorkPacket> returnPackage)
	{
		
		return false;
	}

	void LocalAccessManager::HandleLocationRetAction(shared_ptr<NetWorkPacket> returnPackage)
	{
		
	}

	void LocalAccessManager::HandleLocationCallAction(shared_ptr<NetWorkPacket> returnPackage)
	{
		const std::string & action = returnPackage->action();
		//TODO
		shared_ptr<NetLuaAction> luaAction; //this->mLocalActionManager->GetLuaAction(action);
		if (luaAction != nullptr)
		{
			XCode code = luaAction->Invoke(returnPackage);
			const long long operatorId = returnPackage->operator_id();
			const long long callbackId = returnPackage->callback_id();
			if (code != XCode::LuaCoroutineReturn && callbackId != 0)
			{
				shared_ptr<NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();

				returnPacket->set_error_code(code);
				returnPacket->set_callback_id(callbackId);
				returnPacket->set_operator_id(operatorId);
				this->mLocalRetActionMessage.push(returnPacket);
			}
			return;
		}
	}
}
