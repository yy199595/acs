#include "LocalService.h"
#include <Manager/ActionManager.h>
#include <Manager/NetSessionManager.h>
#include <Coroutine/CoroutineManager.h>
namespace Sentry
{
	LocalService::LocalService()
	{
	}

	bool LocalService::HasMethod(const std::string &action)
	{
		auto iter = this->mActionMap.find(action);
		return iter != this->mActionMap.end();
	}

	bool LocalService::OnInit()
	{
		SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetSessionManager>());
		return ServiceBase::OnInit();
	}

	void LocalService::GetServiceList(std::vector<shared_ptr<LocalActionProxy>>& service)
	{
		auto iter = this->mActionMap.begin();
		for (; iter != this->mActionMap.end(); iter++)
		{
			service.push_back(iter->second);
		}
	}

	XCode LocalService::InvokeMethod(com::NetWorkPacket * msgData)
	{
		const std::string & method = msgData->method();
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return XCode::CallFunctionNotExist;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		return localAction->Invoke(msgData);
	}

	XCode LocalService::InvokeMethod(const std::string &address, com::NetWorkPacket * msgData)
	{
		const std::string & method = msgData->method();
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return XCode::CallFunctionNotExist;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		return localAction->Invoke(msgData);
	}

	bool LocalService::BindFunction(std::string name, LocalAction1 action)
	{
		return this->BindFunction(name, make_shared<LocalActionProxy1>(action, this->GetServiceName(), name));
	}
	bool LocalService::BindFunction(const std::string &name, shared_ptr<LocalActionProxy> actionBox)
	{
		auto iter = this->mActionMap.find(name);
		if (iter != this->mActionMap.end())
		{
			SayNoDebugError("register " << this->GetTypeName() << "." << name << " fail");
			return false;
		}
		this->mActionMap.emplace(name, actionBox);
		return true;
	}
}