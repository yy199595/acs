#include"ServiceComponent.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
#include"Util/StringHelper.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Script/Extension/Service/LuaService.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
#ifdef __RPC_DEBUG_LOG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	ServiceComponent::ServiceComponent()
	{
		this->mConfig = nullptr;
	}
	bool ServiceComponent::LateAwake()
	{
		assert(this->mConfig);
		this->mRpcComponent = this->GetComponent<RpcHandlerComponent>();
		this->mClientComponent = this->GetComponent<RpcClientComponent>();
		return this->GetConfig().GetListener("rpc", this->mLocalAddress);
	}

	void ServiceComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<ServiceComponent>();
		luaRegister.PushExtensionFunction("Call", Lua::Service::Call);
	}

	std::shared_ptr<com::Rpc::Request> ServiceComponent::NewRpcRequest(const std::string& func, long long userId)
	{
		const RpcInterfaceConfig * interfaceConfig = this->mConfig->GetConfig(func);
		if (interfaceConfig == nullptr)
		{
			LOG_ERROR("not find rpc config : "
				<< fmt::format("{0}.{1}", this->GetName(), func));
			return nullptr;
		}
		if(!interfaceConfig->Request.empty())
		{
			return nullptr;
		}
		std::shared_ptr<com::Rpc::Request> request
			= std::make_shared<com::Rpc::Request>();

		request->set_user_id(userId);
		request->set_func(interfaceConfig->FullName);
		return request;
	}

	std::shared_ptr<com::Rpc::Request> ServiceComponent::NewRpcRequest(const std::string& func, long long userId, const Message& message)
	{
		const RpcInterfaceConfig* protoConfig = this->mConfig->GetConfig(func);
		if (protoConfig == nullptr)
		{
			LOG_ERROR("not find rpc config : "
				<< fmt::format("{0}.{1}", this->GetName(), func));
			return nullptr;
		}
		std::shared_ptr<com::Rpc::Request> request
			= std::make_shared<com::Rpc::Request>();

		request->set_user_id(userId);
		request->mutable_data()->PackFrom(message);
		request->set_func(protoConfig->FullName);
		return request;
	}

}

namespace Sentry
{
	XCode ServiceComponent::Send(const std::string& func, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, message);
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

	XCode ServiceComponent::Send(long long int userId, const string& func, const Message& message)
	{
		std::string address;
		if(!this->mAddressProxy.GetAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->SendRequest(address, rpcRequest);
	}

	XCode ServiceComponent::Send(const std::string& address, const std::string& func, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->SendRequest(address, rpcRequest);
	}

	XCode ServiceComponent::Call(const std::string & address, const string& func)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode ServiceComponent::Call(const std::string & address, const string& func, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode ServiceComponent::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0);
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
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, message);
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
		if(this->SendRequest(address, request) != XCode::Successful)
		{
			return XCode::NetWorkError;
		}
		this->mRpcComponent->AddRpcTask(taskSource);
		std::shared_ptr<com::Rpc::Response> responsedata = taskSource->Await();
		if(responsedata == nullptr)
		{
			return XCode::CallTimeout;
		}
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
//		const ServiceConfig& serviceConfig = this->GetApp()->GetServiceConfig();
//		const RpcInterfaceConfig * protoConfig = serviceConfig.GetInterfaceConfig(request->method_id());
//		//LOG_INFO("start call " << protoConfig->FullName);
//		if(protoConfig->CallWay == "Sub") //通过redis 的发布订阅发送
//		{
//			std::string message = "+";
//			request->set_address(this->mLocalAddress);
//			if(request->AppendToString(&message))
//			{
//				long long num = this->mRedisComponent->Publish(address, message);
//				return num == 1 ? XCode::Successful : XCode::NetWorkError;
//			}
//			return XCode::SerializationFailure;
//		}
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
			if(count >= 10)
			{
				return XCode::NetWorkError;
			}
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
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId);
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
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, message);
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
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId);
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
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}
	bool ServiceComponent::SocketIsOpen(const string& address)
	{
		std::shared_ptr<ServerClientContext> clientContext = this->mClientComponent->GetSession(address);
		return clientContext != nullptr && clientContext->IsOpen();
	}
	bool ServiceComponent::LoadConfig(const rapidjson::Value& json)
	{
		if(this->mConfig == nullptr)
		{
			this->mConfig = new RpcServiceConfig(this->GetName());
		}
		return this->mConfig->OnLoadConfig(json);
	}
}
