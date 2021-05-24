#include"LocalService.h"
#include<Manager/ActionManager.h>
#include<Manager/NetWorkManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	LocalService::LocalService()
	{
		
	}

	XCode LocalService::CallAction(SharedPacket request, SharedPacket returnData)
	{
		const std::string & action = request->action();
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

	void LocalService::AddActionArgv(const std::string & address, SharedPacket argv)
	{
		SharedNetPacket packet = make_shared<NetMessageBuffer>(address, argv);
		this->mHandleMessageQueue.AddItem(packet);
	}

	bool LocalService::HasAction(const std::string & action)
	{
		auto iter = this->mActionMap.find(action);
		return iter != this->mActionMap.end();
	}

	bool LocalService::OnInit()
	{
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		return ServiceBase::OnInit();
	}

	void LocalService::OnSystemUpdate()
	{
		SharedNetPacket handlePacket;
		this->mHandleMessageQueue.SwapQueueData();
		while (this->mHandleMessageQueue.PopItem(handlePacket))
		{
			const std::string & address = handlePacket->mAddress;
			SharedPacket packetData = handlePacket->mMessagePacket;
			SharedTcpSession tcpSession = this->mNetWorkManager->GetTcpSession(address);
			if (tcpSession != nullptr)
			{
				this->HandleMessage(tcpSession, packetData);
			}
		}
	}

	bool LocalService::HandleMessage(SharedPacket packet)
	{
		const std::string & action = packet->action();
		auto iter = this->mActionMap.find(action);
		if (iter == this->mActionMap.end())
		{
			SayNoDebugError("not find method " << action);
			return false;
		}
		shared_ptr<LocalActionProxy> localAction = iter->second;
		this->mCorManager->Start(action, [this, packet, localAction]()
		{
			SharedPacket retData = make_shared<NetWorkPacket>();
			XCode code = localAction->Invoke(packet, retData);
			if (packet->callback_id() != 0)
			{
				retData->set_error_code(code);
				retData->set_callback_id(packet->callback_id());
				retData->set_operator_id(packet->operator_id());
				this->mActionManager->AddActionArgv(retData);
			}
		});
		return true;
	}

	bool LocalService::HandleMessage(SharedTcpSession session, SharedPacket packet)
	{
		const std::string & action = packet->action();
		auto iter = this->mActionMap.find(action);
		if (iter == this->mActionMap.end())
		{
			SayNoDebugError("not find method " << action);
			return false;
		}
		const std::string & address = session->GetAddress();
		shared_ptr<LocalActionProxy> localAction = iter->second;
		this->mCorManager->Start(action, [address, this, packet, localAction]()
		{
			SharedPacket retData = make_shared<NetWorkPacket>();
			XCode code = localAction->Invoke(packet, retData);
			if (packet->callback_id() != 0)
			{
				retData->set_error_code(code);
				retData->set_callback_id(packet->callback_id());
				retData->set_operator_id(packet->operator_id());
				this->mNetWorkManager->SendMessageByAdress(address, retData);
			}
		});
		return true;
	}

	bool LocalService::BindFunction(std::string name, LocalAction1 action)
	{
		return this->BindFunction(name, make_shared<LocalActionProxy1>(action, name));
	}
	bool LocalService::BindFunction(const std::string & name, shared_ptr<LocalActionProxy> actionBox)
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