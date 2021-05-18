#include"LocalAccessManager.h"
#include"LocalActionManager.h"
#include<NetWork/NetLuaAction.h>
#include<NetWork/NetWorkAction.h>
#include<NetWork/NetLuaRetAction.h>
#include<NetWork/NetWorkRetAction.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	bool LocalAccessManager::OnInit()
	{
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->mLocalActionManager = this->GetManager<LocalActionManager>());
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

	bool LocalAccessManager::CallAction(const std::string & func, shared_ptr<NetWorkPacket> returnPackage)
	{
		shared_ptr<NetLuaAction> luaAction = this->mLocalActionManager->GetLuaAction(func);
		if (luaAction != nullptr)
		{
			this->mLocalCallActionMessage.push(returnPackage);
			return true;
		}
		shared_ptr<LocalActionProxy> localAction = this->mLocalActionManager->GetAction(func);
		if (localAction != nullptr)
		{
			this->mLocalCallActionMessage.push(returnPackage);
			return true;
		}
		return false;
	}

	void LocalAccessManager::HandleLocationRetAction(shared_ptr<NetWorkPacket> returnPackage)
	{
		const long long callbackId = returnPackage->callback_id();
		shared_ptr<LocalRetActionProxy> callBack = this->mLocalActionManager->GetCallback(callbackId);
		if (callBack != nullptr)
		{
			callBack->Invoke(returnPackage);
		}
	}

	void LocalAccessManager::HandleLocationCallAction(shared_ptr<NetWorkPacket> returnPackage)
	{
		const std::string & action = returnPackage->func_name();

		shared_ptr<NetLuaAction> luaAction = this->mLocalActionManager->GetLuaAction(action);
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
		shared_ptr<LocalActionProxy> localAction = this->mLocalActionManager->GetAction(action);
		if (localAction != nullptr)
		{
			this->mCoroutineManager->Start(action, [this, localAction, returnPackage]()
			{
				const long long callbackId = returnPackage->callback_id();
				const long long operatorId = returnPackage->operator_id();
				shared_ptr<NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();
				XCode code = localAction->Invoke(returnPackage, returnPacket);
				if (callbackId != 0)
				{
					returnPacket->set_error_code(code);
					returnPacket->set_callback_id(callbackId);
					returnPacket->set_operator_id(operatorId);
					this->mLocalRetActionMessage.push(returnPacket);
				}
			});
		}
	}
}
