//
// Created by mac on 2022/4/6.
//

#include"ServiceNode.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Rpc/RpcComponent.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
namespace Sentry
{

	ServiceNode::ServiceNode(const std::string & name)
		: mService(name)
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
		this->mRpcComponent = App::Get()->GetComponent<RpcComponent>();
		this->mRpcConfigComponent = App::Get()->GetComponent<RpcConfigComponent>();
		this->mRpcClientComponent = App::Get()->GetComponent<RpcClientComponent>();
	}

	ServiceNode::ServiceNode(const std::string & name, const string& address)
		: mService(name), mAddress(address)
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
		this->mRpcComponent = App::Get()->GetComponent<RpcComponent>();
		Helper::String::ParseIpAddress(address, this->mIp, this->mPort);
		this->mRpcConfigComponent = App::Get()->GetComponent<RpcConfigComponent>();
	}

	std::shared_ptr<com::Rpc::Request> ServiceNode::NewRequestData(const std::string& func)
	{
		std::string name = fmt::format("{0}.{1}", this->mService, func);
		const ProtoConfig * protoConfig = this->mRpcConfigComponent->GetProtocolConfig(name);
		if(protoConfig == nullptr)
		{
			LOG_ERROR("not find service method = {0}", name);
			return nullptr;
		}
		std::shared_ptr<com::Rpc::Request> rpcRequest(new com::Rpc::Request());
		rpcRequest->set_method_id(protoConfig->MethodId);
		return rpcRequest;
	}

	XCode ServiceNode::Call(const string& func)
	{
		return this->Call(func, 0);
	}

	XCode ServiceNode::Call(const string& func, const Message& message)
	{
		return this->Call(func, 0, message);
	}

	XCode ServiceNode::Call(const string& func, std::shared_ptr<Message> response)
	{
		return this->Call(func, 0, response);
	}

	XCode ServiceNode::Call(const string& func, const Message& message, std::shared_ptr<Message> response)
	{
		return this->Call(func, 0, message, response);
	}

	XCode ServiceNode::Call(const string& func, long long int userId)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRequestData(func);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		rpcRequest->set_user_id(userId);
		if(this->mAddress.empty())
		{
			return this->mRpcComponent->OnLocalRequest(rpcRequest, nullptr);
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode ServiceNode::Call(const string& func, long long int userId, const Message& message)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRequestData(func);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		rpcRequest->set_user_id(userId);
		rpcRequest->mutable_data()->PackFrom(message);
		if(this->mAddress.empty())
		{
			return this->mRpcComponent->OnLocalRequest(rpcRequest, nullptr);
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(rpcRequest);
		return rpcResponse != nullptr ? (XCode)rpcResponse->code() : XCode::NetWorkError;
	}

	XCode ServiceNode::Call(const string& func, long long int userId, std::shared_ptr<Message> response)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRequestData(func);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		rpcRequest->set_user_id(userId);
		if(this->mAddress.empty())
		{
			std::shared_ptr<com::Rpc::Response> rpcResponse(new com::Rpc::Response());
			XCode code = this->mRpcComponent->OnLocalRequest(rpcRequest, rpcResponse);
			if(code == XCode::Successful && rpcResponse->has_data())
			{
				rpcResponse->mutable_data()->UnpackTo(response.get());
			}
			return code;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(rpcRequest);
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

	XCode ServiceNode::Call(const string& func, long long int userId, const Message& message,
			std::shared_ptr<Message> response)
	{
		std::shared_ptr<com::Rpc::Request> rpcRequest = this->NewRequestData(func);
		if(rpcRequest == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		rpcRequest->set_user_id(userId);
		rpcRequest->mutable_data()->PackFrom(message);
		if(this->mAddress.empty())
		{
			std::shared_ptr<com::Rpc::Response> rpcResponse(new com::Rpc::Response());
			XCode code = this->mRpcComponent->OnLocalRequest(rpcRequest, rpcResponse);
			if(code == XCode::Successful && rpcResponse->has_data())
			{
				rpcResponse->mutable_data()->UnpackTo(response.get());
			}
			return code;
		}
		std::shared_ptr<com::Rpc::Response> rpcResponse = this->StartCall(rpcRequest);
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

	std::shared_ptr<com::Rpc::Response> ServiceNode::StartCall(std::shared_ptr<com::Rpc::Request> request)
	{
		std::shared_ptr<ProtoRpcClient> rpcClient = this->mRpcClientComponent->GetSession(this->mAddress);
		if(rpcClient == nullptr)
		{
			rpcClient = this->mRpcClientComponent->MakeSession(this->mService, this->mAddress);
			for(size_t index = 0; index < 3; index++)
			{
				if(rpcClient->ConnectAsync(this->mIp, this->mPort)->Await())
				{
					break;
				}
				this->mTaskComponent->Sleep(2000);
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
}