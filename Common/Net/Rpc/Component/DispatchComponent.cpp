#include"DispatchComponent.h"

#include"Async/Component/CoroutineComponent.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Timer/Timer/ElapsedTimer.h"

#ifdef __DEBUG__
#include"Proto/Component/ProtoComponent.h"
#endif

#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Common/Message.h"
#include"Rpc/Service/RpcService.h"
#include"Router/Component/RouterComponent.h"

#include"Core/System/System.h"

namespace acs
{
	DispatchComponent::DispatchComponent()
	{
		this->mSumCount = 0;
		this->mWaitCount = 0;
		this->mRouter = nullptr;
		this->mCoroutine = nullptr;
		this->mGateSender = nullptr;
	}

	bool DispatchComponent::LateAwake()
	{
		std::vector<RpcService*> rpcServices;
		this->mApp->GetComponents(rpcServices);
		for (RpcService* rpcService: rpcServices)
		{
			const std::string& name = rpcService->GetName();
			LOG_CHECK_RET_FALSE(this->mRpcServices.Add(name, rpcService));
		}
		this->mGateSender = this->mApp->GetComponent<rpc::IOuterSender>();
		LOG_CHECK_RET_FALSE(this->mRouter = this->GetComponent<RouterComponent>())
		LOG_CHECK_RET_FALSE(this->mCoroutine = this->GetComponent<CoroutineComponent>())
		return true;
	}

	void DispatchComponent::OnAppStop()
	{
		while (this->mWaitCount > 1)
		{
			this->mApp->Sleep();
			LOG_INFO("wait message count:{}", this->mWaitCount);
		}
	}

	int DispatchComponent::OnRequest(rpc::Message* message) noexcept
	{
		++this->mSumCount;
		int code = XCode::Ok;
		const std::string& fullName = message->ConstHead().GetStr(rpc::Header::func);
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);

		do
		{
			if (methodConfig == nullptr)
			{
				code = XCode::CallFunctionNotExist;
				break;
			}
//#ifdef __DEBUG__
//			std::string fromAddress = "unknown";
//			message->TempHead().Del(rpc::Header::from_addr, fromAddress);
//			LOG_DEBUG("[{}://{}] call ({})", methodConfig->NetName, fromAddress, methodConfig->FullName);
//#endif
			//LOG_DEBUG("call ({}) by {}", fullName, methodConfig->NetName);
			if (!this->mRpcServices.Has(methodConfig->service))
			{
				code = XCode::CallServiceNotFound;
				LOG_ERROR("call [{}] not exist", methodConfig->service);
				break;
			}
			if (!methodConfig->async)
			{
				this->Invoke(methodConfig, message);
				break;
			}
			this->mCoroutine->Start(&DispatchComponent::Invoke, this, methodConfig, message);
		}
		while (false);
		if(code != XCode::Ok)
		{
			int id = message->SockId();
			this->mRouter->Send(id, code, message);
			return XCode::Ok;
		}
		return code;
	}

	void DispatchComponent::Invoke(const RpcMethodConfig* config, rpc::Message* message) noexcept
	{
		++this->mWaitCount;
		int code = XCode::Ok;
#ifdef __DEBUG__
		long long start = help::Time::NowMil();
#endif
		do
		{
			const std::string& service = config->service;
			RpcService* logicService = this->mRpcServices.Find(service);
			if (logicService == nullptr)
			{
				code = XCode::CallServiceNotFound;
				LOG_ERROR("call {} server not found", config->fullname);
				break;
			}
			code = logicService->Invoke(config, message);
		} while (false);
#ifdef __DEBUG__
		long long t = help::Time::NowMil() - start;
//		if (code != XCode::Ok)
//		{
//			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
//			LOG_WARN("({}ms) invoke [{}] code:{} = {}", t, config->FullName, code, desc);
//		}
		if (config->timeout > 0 && t >= config->timeout)
		{
			LOG_WARN("({}ms) invoke [{}] too long time", t, config->fullname);
		}
#endif
		--this->mWaitCount;
		this->mRouter->Send(message->SockId(), code, message);
	}

	void DispatchComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("dispatch");
		{
			data->Add("sum", this->mSumCount);
			data->Add("wait", this->mWaitCount);
			data->Add("rpc_wait", this->AwaitCount());
		}
	}

	int DispatchComponent::OnMessage(rpc::Message* message) noexcept
	{
		switch (message->GetType())
		{
			case rpc::Type::Request:
				return this->OnRequest(message);
			case rpc::Type::Response:
			{
				if(message->GetSource() == rpc::Source::Client)
				{
					if (this->mGateSender != nullptr)
					{
						int socketId = 0;
						message->GetHead().Get(rpc::Header::sock_id, socketId);
						this->mGateSender->Send(socketId, message);
						return XCode::Ok;
					}
				}
				int rpcId = message->GetRpcId();
				this->OnResponse(rpcId, std::unique_ptr<rpc::Message>(message));
				return XCode::Ok;
			}
			case rpc::Type::Client:
				return this->OnClient(message);
			case rpc::Type::Broadcast:
				return this->OnBroadcast(message);
		}
		LOG_ERROR("unknown message type : {}", message->GetType());
		return XCode::UnKnowPacket;
	}

	int DispatchComponent::OnClient(rpc::Message* message)
	{
		if (this->mGateSender == nullptr)
		{
			return XCode::NetWorkError;
		}
		int sockId = 0;
		if (!message->GetHead().Get(rpc::Header::client_sock_id, sockId))
		{
			LOG_ERROR("not find userid forward to client fail");
			return XCode::CallArgsError;
		}
		message->SetType(rpc::Type::Request);
		message->GetHead().Del(rpc::Header::client_sock_id);
		this->mGateSender->Send(sockId, message);
		return XCode::Ok;
	}

	int DispatchComponent::OnBroadcast(rpc::Message* message)
	{
		if (this->mGateSender == nullptr)
		{
			return XCode::Failure;
		}
		message->SetType(rpc::Type::Request);
		this->mGateSender->Broadcast(message);
		return XCode::Ok;
	}
}// namespace Sentry
