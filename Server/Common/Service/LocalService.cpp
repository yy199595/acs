#include "LocalService.h"
#include <Manager/ActionManager.h>
#include <Manager/NetWorkManager.h>
#include <Coroutine/CoroutineManager.h>
namespace SoEasy
{
	LocalService::LocalService()
	{
	}

	SharedTcpSession LocalService::GetCurTcpSession()
	{
		long long currentId = this->mCorManager->GetCurrentCorId();
		auto iter = this->mCurrentSessionMap.find(currentId);
		if (iter != this->mCurrentSessionMap.end())
		{
			const std::string &address = iter->second;
			return this->mNetWorkManager->GetTcpSession(address);
		}
		return nullptr;
	}

	XCode LocalService::CallAction(SharedPacket request, SharedPacket returnData)
	{
		const std::string &action = request->method();
		auto iter = this->mActionMap.find(action);
		if (iter == this->mActionMap.end())
		{
			SayNoDebugError("call func not find " << this->GetTypeName()
												  << "." << action);
			return XCode::CallFunctionNotExist;
		}
		shared_ptr<LocalActionProxy> actionProxy = iter->second;
		return actionProxy->Invoke(request, returnData);
	}

	bool LocalService::HasMethod(const std::string &action)
	{
		auto iter = this->mActionMap.find(action);
		return iter != this->mActionMap.end();
	}

	bool LocalService::OnInit()
	{
		SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		return ServiceBase::OnInit();
	}

	XCode LocalService::InvokeMethod(const SharedPacket requestData, SharedPacket responseData)
	{
		const std::string & method = requestData->method();
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return XCode::CallFunctionNotExist;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		return localAction->Invoke(requestData, responseData);
	}

	XCode LocalService::InvokeMethod(const std::string &address, const SharedPacket requestData, SharedPacket responseData)
	{
		const std::string & method = requestData->method();
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return XCode::CallFunctionNotExist;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		return localAction->Invoke(requestData, responseData);
	}

	bool LocalService::BindFunction(std::string name, LocalAction1 action)
	{
		return this->BindFunction(name, make_shared<LocalActionProxy1>(action, name));
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

	bool LocalService::Bind(std::string name, MysqlOperAction action)
	{
		return this->BindFunction(name, make_shared<LocalMysqlActionProxy>(action, name));
	}

	bool LocalService::Bind(std::string name, MysqlQueryAction action)
	{
		return this->BindFunction(name, make_shared<LocalMysqlQueryActionProxy>(action, name));
	}
}