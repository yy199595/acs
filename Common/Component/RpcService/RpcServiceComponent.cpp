#include"RpcServiceComponent.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/RpcConfig.h"
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#endif
#include"Util/StringHelper.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Rpc/RpcComponent.h"
#include"Component/Rpc/RpcClientComponent.h"

namespace Sentry
{
	ServiceRecord::ServiceRecord()
	{
		this->mCallCount = 0;
		this->mStartTime = 0;
	}

	void ServiceRecord::OnCall(long long ms)
	{
		this->mCallCount++;
		this->mStartTime += ms;
	}
	long long ServiceRecord::GetWeight()
	{
		return this->mStartTime / this->mCallCount;
	}
}

namespace Sentry
{
	bool RpcServiceComponent::LateAwake()
	{
		this->mRpcComponent = this->GetComponent<RpcComponent>();
		this->mRpcClientComponent = this->GetComponent<RpcClientComponent>();
		return true;
	}

	std::shared_ptr<com::Rpc::Request> RpcServiceComponent::NewRpcRequest(const std::string& func, long long userId, const Message* message)
	{
		const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
		string name = fmt::format("{0}.{1}", this->GetName(), func);
		const ProtoConfig * protoConfig = rpcConfig.GetProtocolConfig(name);
		if(protoConfig == nullptr)
		{
			LOG_ERROR("not find rpc config : {0}", name);
			return nullptr;
		}
		std::shared_ptr<com::Rpc::Request> request(new com::Rpc::Request());
		request->set_user_id(userId);
		request->set_method_id(protoConfig->MethodId);
		if(message != nullptr)
		{
			request->mutable_data()->PackFrom(*message);
		}
		return request;
	}
}

namespace Sentry
{
	XCode RpcServiceComponent::Call(const std::string & address, const string& func)
	{
		return this->Call(address, func, 0);
	}

	XCode RpcServiceComponent::Call(const std::string & address, const string& func, const Message& message)
	{
		return this->Call(address, func, 0, message);
	}

	XCode RpcServiceComponent::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		return this->Call(address, func, 0, response);
	}

	XCode RpcServiceComponent::Call(const std::string & address, const string& func, const Message& message,
			std::shared_ptr<Message> response)
	{
		return this->Call(address, func, 0, message, response);
	}

	std::shared_ptr<com::Rpc::Response> RpcServiceComponent::StartCall(const std::string & address,
			std::shared_ptr<com::Rpc::Request> request)
	{
		std::shared_ptr<ProtoRpcClient> rpcClient = this->mRpcClientComponent->GetOrCreateSession("rpc", address);
		if(!rpcClient->IsConnected())
		{
			std::string ip;
			unsigned short port = 0;
			Helper::String::ParseIpAddress(address, ip, port);
			for(size_t index = 0; index < 3; index++)
			{
				if(rpcClient->ConnectAsync(ip, port)->Await())
				{
					break;
				}
				App::Get()->GetTaskComponent()->Sleep(2000);
			}
			if(!rpcClient->IsConnected())
			{
				return nullptr;
			}
		}
		std::shared_ptr<RpcTaskSource> taskSource(new RpcTaskSource());

		request->set_rpc_id(taskSource->GetRpcId());
		this->mRpcComponent->AddRpcTask(taskSource);
#ifdef __DEBUG__
		this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), request->method_id());
#endif
		rpcClient->SendToServer(request);
		return taskSource->Await();
	}

	void RpcServiceComponent::AddRemoteAddress(const string& address)
	{

	}

	void RpcServiceComponent::DelRemoteAddress(const string& address)
	{

	}
}

namespace Sentry
{
	XCode RpcServiceComponent::Call(const std::string& address, const std::string& func, long long userId)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode RpcServiceComponent::Call(const std::string& address, const std::string& func, long long userId, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode RpcServiceComponent::Call(const std::string& address, const std::string& func, long long userId,
			std::shared_ptr<Message> response)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		if(rpcResponse == nullptr)
		{
			return XCode::NetWorkError;
		}
		if(rpcResponse->code() == (int)XCode::Successful && rpcResponse->has_data())
		{
			rpcResponse->mutable_data()->UnpackTo(response.get());
		}
		return (XCode)rpcResponse->code();
	}

	XCode RpcServiceComponent::Call(const std::string& address, const std::string& func,
			long long userId, const Message& message, std::shared_ptr<Message> response)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		if(rpcResponse == nullptr)
		{
			return XCode::NetWorkError;
		}
		if(rpcResponse->code() == (int)XCode::Successful && rpcResponse->has_data())
		{
			rpcResponse->mutable_data()->UnpackTo(response.get());
		}
		return (XCode)rpcResponse->code();
	}
}
