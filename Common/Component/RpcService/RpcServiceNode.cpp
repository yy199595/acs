#include"RpcServiceNode.h"
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
#include"Component/RpcService/LocalServerRpc.h"

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
	bool RpcServiceNode::LateAwake()
	{
		this->mRpcComponent = this->GetComponent<RpcComponent>();
		this->mRpcClientComponent = this->GetComponent<RpcClientComponent>();
		return true;
	}

	Component* RpcServiceNode::Copy(const std::string& name)
	{
		Component * component = ComponentFactory::CreateComponent(name);
		LocalServerRpc * localServerRpc = component->Cast<LocalServerRpc>();
		if(localServerRpc == nullptr)
		{
			return nullptr;
		}
		localServerRpc->mRpcComponent = this->mRpcComponent;
		localServerRpc->mUserAddressMap = this->mUserAddressMap;
		localServerRpc->mRemoteAddressList = this->mRemoteAddressList;
		localServerRpc->mRpcClientComponent = this->mRpcClientComponent;
		return localServerRpc;
	}

	std::shared_ptr<com::Rpc::Request> RpcServiceNode::NewRpcRequest(const std::string& func, long long userId, const Message* message)
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
	XCode RpcServiceNode::Call(const std::string & address, const string& func)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode RpcServiceNode::Call(const std::string & address, const string& func, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode RpcServiceNode::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, nullptr);
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

	XCode RpcServiceNode::Call(const std::string & address, const string& func, const Message& message,
			std::shared_ptr<Message> response)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, 0, &message);
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

	std::shared_ptr<com::Rpc::Response> RpcServiceNode::StartCall(const std::string & address,
			std::shared_ptr<com::Rpc::Request> request)
	{
		std::shared_ptr<ProtoRpcClient> rpcClient = this->mRpcClientComponent->GetOrCreateSession("rpc", address);

		if(rpcClient == nullptr)
		{
			return nullptr;
		}

		if(!rpcClient->IsOpen())
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
			if(!rpcClient->IsOpen())
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

	bool RpcServiceNode::AllotAddress(std::string & address)
	{
		if(this->mRemoteAddressList.empty())
		{
			return false;
		}
		auto iter = this->mRemoteAddressList.begin();
		for(; iter != this->mRemoteAddressList.end(); iter++)
		{
			address = *iter;
			return true;
		}
		return false;
	}

	void RpcServiceNode::AddEntity(long long id)
	{
		std::string address;
		if(this->AllotAddress(address))
		{
			auto iter = this->mUserAddressMap.find(id);
			if(iter != this->mUserAddressMap.end())
			{
				this->mUserAddressMap.erase(iter);
			}
			this->mUserAddressMap.emplace(id, address);
		}
	}

	void RpcServiceNode::DelEntity(long long id)
	{
		auto iter = this->mUserAddressMap.find(id);
		if(iter != this->mUserAddressMap.end())
		{
			this->mUserAddressMap.erase(iter);
		}
	}

	void RpcServiceNode::AddAddress(const string& address)
	{
		this->mRemoteAddressList.insert(address);
		LOG_ERROR("{0} add address {1}", this->GetName(), address);
	}

	void RpcServiceNode::DelAddress(const string& address)
	{
		auto iter = this->mRemoteAddressList.find(address);
		if(iter != this->mRemoteAddressList.end())
		{
			this->mRemoteAddressList.erase(iter);
			LOG_WARN("{0} delete address {1}", this->GetName(), address);
		}
	}
}

namespace Sentry
{
	XCode RpcServiceNode::Call(const std::string& func, long long userId)
	{
		std::string address;
		if(this->GetEntityAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, nullptr);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode RpcServiceNode::Call(const std::string& func, long long userId, const Message& message)
	{
		std::string address;
		if(this->GetEntityAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRpcRequest(func, userId, &message);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(address, rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode RpcServiceNode::Call(const std::string& func, long long userId,
			std::shared_ptr<Message> response)
	{
		std::string address;
		if(this->GetEntityAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
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

	XCode RpcServiceNode::Call(const std::string& func, long long userId, const Message& message, std::shared_ptr<Message> response)
	{
		std::string address;
		if(this->GetEntityAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
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

	bool RpcServiceNode::GetEntityAddress(long long int id, string& address)
	{
		auto iter = this->mUserAddressMap.find(id);
		if(iter != this->mUserAddressMap.end())
		{
			auto iter1 = this->mRemoteAddressList.find(iter->second);
			if(iter1 != this->mRemoteAddressList.end())
			{
				address = iter->second;
				return true;
			}
		}
		return false;
	}

}
