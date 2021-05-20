#include"ServiceBase.h"
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	ServiceBase::ServiceBase()
	{
		this->mNetWorkManager = nullptr;
		this->mCoroutineManager = nullptr;
	}
	XCode ServiceBase::CallAction(SharedPacket request, SharedPacket returnData)
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

	void ServiceBase::AddActionArgv(SharedPacket argv)
	{
		this->mLocalMessageQueue.push(argv);
	}

	void ServiceBase::AddActionArgv(SharedNetPacket argv)
	{
		this->mHandleMessageQueue.AddItem(argv);
	}

	bool ServiceBase::OnInit()
	{
		SayNoAssertRetFalse_F(this->mActionManager = this->GetManager<ActionManager>());
		SayNoAssertRetFalse_F(this->mNetWorkManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());

		return true;
	}

	void ServiceBase::OnSystemUpdate()
	{
		SharedNetPacket handlePacket;
		this->mHandleMessageQueue.SwapQueueData();
		while (this->mHandleMessageQueue.PopItem(handlePacket))
		{
			const std::string & address = handlePacket->mAddress;
			SharedTcpSession tcpSession = this->mNetWorkManager->GetTcpSession(address);
			if (tcpSession != nullptr)
			{
				SharedPacket packetData = handlePacket->mMessagePacket;
				const std::string & action = packetData->action();
				this->mCoroutineManager->Start(action, [packetData, address, this]()
				{
					SharedPacket retData = make_shared<NetWorkPacket>();
					XCode code = this->CallAction(packetData, retData);
					if (packetData->callback_id() != 0)
					{
						retData->set_callback_id(packetData->callback_id());
						retData->set_operator_id(packetData->operator_id());
						this->mNetWorkManager->SendMessageByAdress(address, retData);
					}
				});
			}
		}
		while (!this->mLocalMessageQueue.empty())
		{
			SharedPacket packetData = this->mLocalMessageQueue.front();
			this->mLocalMessageQueue.pop();
			const std::string & action = packetData->action();
			this->mCoroutineManager->Start(action, [packetData, this]()
			{
				SharedPacket retData = make_shared<NetWorkPacket>();
				XCode code = this->CallAction(packetData, retData);
				if (packetData->callback_id() != 0)
				{
					retData->set_callback_id(packetData->callback_id());
					retData->set_operator_id(packetData->operator_id());
					this->mActionManager->AddCallbackArgv(retData);
				}
			});
		}
	}

	bool ServiceBase::BindFunction(std::string name, LocalAction1 action)
	{
		return this->BindFunction(name, make_shared<LocalActionProxy1>(action, name));
	}
	bool ServiceBase::BindFunction(const std::string & name, shared_ptr<LocalActionProxy> actionBox)
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