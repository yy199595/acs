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

	bool LocalService::InvokeMethod(const std::string &method, shared_ptr<NetWorkPacket> reqData)
	{
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return false;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		this->Start(method, [localAction, reqData, this]()
					{
						shared_ptr<NetWorkPacket> resData = make_shared<NetWorkPacket>();
						XCode code = localAction->Invoke(reqData, resData);
						if (reqData->rpcid() != 0)
						{
							resData->set_code(code);
							resData->set_rpcid(reqData->rpcid());
							resData->set_entityid(reqData->entityid());
							this->ReplyMessage(reqData->rpcid(), resData);
						}
					});
		return true;
	}

	bool LocalService::InvokeMethod(const std::string &address, const std::string &method, SharedPacket reqData)
	{
		auto iter = this->mActionMap.find(method);
		if (iter == this->mActionMap.end())
		{
			return false;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		this->mCorManager->Start(method, [address, this, reqData, localAction]()
								 {
									 SharedPacket retData = make_shared<NetWorkPacket>();
									 long long currentId = this->mCorManager->GetCurrentCorId();
									 this->mCurrentSessionMap.emplace(currentId, address);
									 XCode code = localAction->Invoke(reqData, retData);
									 if (reqData->rpcid() != 0)
									 {
										 retData->set_code(code);
										 retData->set_rpcid(reqData->rpcid());
										 retData->set_entityid(reqData->entityid());
										 this->ReplyMessage(address, retData);
									 }
									 auto iter = mCurrentSessionMap.find(currentId);
									 if (iter != mCurrentSessionMap.end())
									 {
										 mCurrentSessionMap.erase(iter);
									 }
								 });
		return true;
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