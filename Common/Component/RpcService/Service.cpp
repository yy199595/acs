#include"Service.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
#include"Util/StringHelper.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Script/Extension/Service/LuaService.h"
#include"Component/Rpc/InnerNetMessageComponent.h"
#include"Component/Rpc/InnerNetComponent.h"
#ifdef __RPC_DEBUG_LOG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	Service::Service()
	{
		this->mConfig = nullptr;
	}
	bool Service::LateAwake()
	{
		assert(this->mConfig);
		this->mClientComponent = this->GetComponent<InnerNetComponent>();
        this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>();
        return this->GetConfig().GetListener("rpc", this->mLocalAddress);
	}

	void Service::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<Service>();
		luaRegister.PushExtensionFunction("Call", Lua::Service::Call);
		luaRegister.PushExtensionFunction("GetHost", Lua::Service::GetHost);
	}

	std::shared_ptr<com::rpc::request> Service::NewRpcRequest(const std::string& func, long long userId)
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
		std::shared_ptr<com::rpc::request> request
			= std::make_shared<com::rpc::request>();

		request->set_user_id(userId);
		request->set_func(interfaceConfig->FullName);
		return request;
	}

	std::shared_ptr<com::rpc::request> Service::NewRpcRequest(const std::string& func, long long userId, const Message& message)
	{
		const RpcInterfaceConfig* protoConfig = this->mConfig->GetConfig(func);
		if (protoConfig == nullptr)
		{
			LOG_ERROR("not find rpc config : "
				<< fmt::format("{0}.{1}", this->GetName(), func));
			return nullptr;
		}
		std::shared_ptr<com::rpc::request> request
			= std::make_shared<com::rpc::request>();

		request->set_user_id(userId);
		request->mutable_data()->PackFrom(message);
		request->set_func(protoConfig->FullName);
		return request;
	}

}

namespace Sentry
{
	XCode Service::Send(const std::string& func, const Message& message)
	{
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, 0, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
        this->mServiceHosts.clear();
        if(!this->GetHosts(this->mServiceHosts))
        {
            LOG_ERROR("service address list empty");
            return XCode::CallServiceNotFound;
        }
		for(const std::string & address : this->mServiceHosts)
		{
			if(this->SendRequest(address, rpcRequest) != XCode::Successful)
			{
				LOG_ERROR("send to " << address << "failure");
			}
		}
		return XCode::Successful;
	}

	XCode Service::Send(long long int userId, const string& func, const Message& message)
	{
		std::string address;
		if(!this->GetHost(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, userId, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->SendRequest(address, rpcRequest);
	}

	XCode Service::Send(const std::string& address, const std::string& func, const Message& message)
	{
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, 0, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->SendRequest(address, rpcRequest);
	}

	XCode Service::Call(const std::string & address, const string& func)
	{
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, 0);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode Service::Call(const std::string & address, const string& func, const Message& message)
	{
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, 0, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode Service::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, 0);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}


	XCode Service::Call(const std::string & address, const string& func, const Message& message,
			std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, 0, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}

	XCode Service::Call(long long userId, std::shared_ptr<com::rpc::request> request, std::shared_ptr<Message> response)
	{
		std::string address;
		if(!this->GetHost(userId, address))
		{
			return XCode::NotFindUser;
		}
		return this->Call(address, request, response);
	}

	XCode Service::Call(const std::string& address, std::shared_ptr<com::rpc::request> request, std::shared_ptr<Message> response)
	{
		std::shared_ptr<RpcTaskSource> taskSource =
			std::make_shared<RpcTaskSource>(0);
		request->set_rpc_id(taskSource->GetRpcId());
		this->mMessageComponent->AddTask(taskSource);
		if(this->SendRequest(address, request) != XCode::Successful)
		{
			return XCode::NetWorkError;
		}
		std::shared_ptr<com::rpc::response> responsedata = taskSource->Await();
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

	XCode Service::SendRequest(const std::string& address, std::shared_ptr<com::rpc::request> request)
	{
		if(!this->HasHost(address))
		{
			return XCode::NetWorkError;
		}
		this->mClientComponent->Send(address, request);
		return XCode::Successful;
	}
}

namespace Sentry
{
	XCode Service::Call(long long userId, const std::string& func)
	{
		std::string address;
		if(!this->GetHost(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, userId);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode Service::Call(long long userId, const std::string& func, const Message& message)
	{
		std::string address;
		if(!this->GetHost(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, userId, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, nullptr);
	}

	XCode Service::Call(long long userId, const std::string& func, std::shared_ptr<Message> response)
	{
		std::string address;
		assert(response != nullptr);
		if(!this->GetHost(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, userId);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}

	XCode Service::Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response)
	{
		std::string address;
		assert(response != nullptr);
		if(!this->GetHost(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::rpc::request> rpcRequest = this->NewRpcRequest(func, userId, message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->Call(address, rpcRequest, response);
	}

	bool Service::LoadConfig(const rapidjson::Value& json)
	{
		if(this->mConfig == nullptr)
		{
			this->mConfig = new RpcServiceConfig(this->GetName());
		}
		return this->mConfig->OnLoadConfig(json);
	}
}
