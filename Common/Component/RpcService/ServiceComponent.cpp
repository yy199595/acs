#include"ServiceComponent.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#endif
#include"Util/StringHelper.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Script/Extension/Service/LuaService.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/Redis/MainRedisComponent.h"

namespace Sentry
{
	bool ServiceComponent::LateAwake()
	{
		this->mRpcComponent = this->GetComponent<RpcHandlerComponent>();
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		this->mClientComponent = this->GetComponent<RpcClientComponent>();
		return this->GetConfig().GetListenerAddress("rpc", this->mLocalAddress);
	}

	void ServiceComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<ServiceComponent>();
		luaRegister.PushExtensionFunction("Call", Lua::Service::Call);
	}

	std::shared_ptr<com::Rpc::Request> ServiceComponent::NewRpcRequest(const std::string& func, long long userId, const Message* message)
	{
		const ServiceConfig& rpcConfig = this->GetApp()->GetServiceConfig();
		const string name = fmt::format("{0}.{1}", this->GetName(), func);
		const RpcInterfaceConfig * protoConfig = rpcConfig.GetInterfaceConfig(name);
		if (protoConfig == nullptr)
		{
			LOG_ERROR("not find rpc config : " << name);
			return nullptr;
		}
		std::shared_ptr<com::Rpc::Request> request
			= std::make_shared<com::Rpc::Request>();

		if(!protoConfig->Request.empty())
		{
			if(message == nullptr)
			{
				LOG_ERROR("call " << protoConfig->FullName << " args error");
				return nullptr;
			}
			request->mutable_data()->PackFrom(*message);
		}
		request->set_user_id(userId);
		request->set_method_id(protoConfig->InterfaceId);
		return request;
	}
}

namespace Sentry
{
	XCode ServiceComponent::Send(const std::string& func, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		for(const std::string & address : this->mAllAddress)
		{
			if(this->SendRequest(address, rpcRequest) != XCode::Successful)
			{
				LOG_ERROR("send to " << address << "failure");
			}
		}
		return XCode::Successful;
	}

	XCode ServiceComponent::Call(const std::string & address, const string& func)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode ServiceComponent::Call(const std::string & address, const string& func, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode ServiceComponent::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}


	XCode ServiceComponent::Call(const std::string & address, const string& func, const Message& message,
			std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}

	XCode ServiceComponent::Call(long long userId, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<Message> response)
	{
		std::string address;
		if(!this->mAddressProxy.GetAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		return this->Call(address, request, response);
	}

	XCode ServiceComponent::Call(const std::string& address, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<Message> response)
	{
		std::shared_ptr<RpcTaskSource> taskSource = std::make_shared<RpcTaskSource>();
		request->set_rpc_id(taskSource->GetRpcId());
		XCode code = this->SendRequest(address, request);
#ifdef __RPC_DEBUG_LOG__
		std::string json;
		ElapsedTimer elapsedTimer;
		const ServiceConfig & config = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig * rpcConfig = config.GetInterfaceConfig(request->method_id());
		LOG_DEBUG("=============== server request ===============");
		LOG_DEBUG("func = " << rpcConfig->FullName);
		LOG_DEBUG("address = " << address);
		if(Helper::Proto::GetJson(request->data(), json))
		{
			LOG_DEBUG("request = " << json);
		}
		LOG_DEBUG("================================================");
#endif
		if(code != XCode::Successful)
		{
			return code;
		}
		this->mRpcComponent->AddRpcTask(taskSource);
		std::shared_ptr<com::Rpc::Response> responsedata = taskSource->Await();
		if(responsedata == nullptr)
		{
#ifdef __RPC_DEBUG_LOG__
			LOG_ERROR(rpcConfig->FullName << " time out");
#endif
			return XCode::CallTimeout;
		}
#ifdef __RPC_DEBUG_LOG__
		LOG_INFO("=============== server response ===============");
		LOG_INFO("func = " << rpcConfig->FullName);
		LOG_INFO("address = " << address);
		LOG_INFO("time = " << elapsedTimer.GetSecond() << "s")
		if(Helper::Proto::GetJson(responsedata->data(), json))
		{
			LOG_INFO("response = " << json);
		}
		LOG_INFO("================================================");
#endif
		if(responsedata->code() == (int)XCode::Successful && response != nullptr)
		{
			if(responsedata->has_data())
			{
				responsedata->mutable_data()->UnpackTo(response.get());
				return XCode::Successful;
			}
		}
		return (XCode)responsedata->code();
	}

	XCode ServiceComponent::SendRequest(const std::string& address, std::shared_ptr<com::Rpc::Request> request)
	{
		const ServiceConfig& serviceConfig = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = serviceConfig.GetInterfaceConfig(request->method_id());
		//LOG_INFO("start call " << protoConfig->FullName);
		if(protoConfig->CallWay == "Sub") //通过redis 的发布订阅发送
		{
			std::string message = "+";
			request->set_address(this->mLocalAddress);
			if(request->AppendToString(&message))
			{
				long long num = this->mRedisComponent->Publish(address, message);
				return num == 1 ? XCode::Successful : XCode::NetWorkError;
			}
			return XCode::SerializationFailure;
		}
		std::shared_ptr<ServerClientContext> clientContext = this->mClientComponent->GetOrCreateSession(address);
		if(clientContext->IsOpen())
		{
			clientContext->SendToServer(request);
			return XCode::Successful;
		}
		int count = 0;
		while(!clientContext->StartConnectAsync())
		{
			this->GetApp()->GetTaskComponent()->Sleep(3000);
			LOG_ERROR("connect [" << address << "] failure count = " << count++);
		}
		clientContext->SendToServer(request);
		return XCode::Successful;
	}
}

namespace Sentry
{
	XCode ServiceComponent::Call(long long userId, const std::string& func)
	{
		std::string address;
		if(!this->mAddressProxy.GetAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode ServiceComponent::Call(long long userId, const std::string& func, const Message& message)
	{
		std::string address;
		if(!this->mAddressProxy.GetAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode ServiceComponent::Call(long long userId, const std::string& func, std::shared_ptr<Message> response)
	{
		std::string address;
		assert(response != nullptr);
		if(!this->mAddressProxy.GetAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}

	XCode ServiceComponent::Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response)
	{
		std::string address;
		assert(response != nullptr);
		if(!this->mAddressProxy.GetAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}
}
