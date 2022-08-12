
#include"RpcClientComponent.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Network/SocketProxy.h"
#include"Component/Rpc/TcpRpcComponent.h"
#include"Global/ServiceConfig.h"
#include"Component/Scene/NetThreadComponent.h"

#include"Async/RpcTask/RpcTaskSource.h"
#include<google/protobuf/util/json_util.h>
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	void RpcClientComponent::Awake()
	{
		this->mRpcComponent = nullptr;
        this->mNetComponent = nullptr;
	}
	bool RpcClientComponent::LateAwake()
	{
        LOG_CHECK_RET_FALSE(this->mNetComponent = this->GetComponent<NetThreadComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<TcpRpcComponent>());
		return true;
	}

	void RpcClientComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			this->mRpcClientMap.erase(iter);
			LOG_WARN("close server address : " << address);
		}
	}

	bool RpcClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		const std::string& address = socket->GetAddress();
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			assert(!address.empty());
			std::shared_ptr<MessageRpcClient> tcpSession
					= std::make_shared<MessageRpcClient>(this, socket);

			tcpSession->StartReceive();
			this->mRpcClientMap.emplace(address, tcpSession);
            return true;
		}
        return false;
	}

	void RpcClientComponent::StartClose(const std::string & address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			auto rpcClient = iter->second;

			rpcClient->StartClose();
			this->mRpcClientMap.erase(iter);
		}
	}

	void RpcClientComponent::OnRequest(std::shared_ptr<com::rpc::request> request)
	{
        assert(this->GetApp()->IsMainThread());
        request->set_type(com::rpc_msg_type_proto);
        const std::string & address = request->address();
		XCode code = this->mRpcComponent->OnRequest(request);
		if (code != XCode::Successful)
		{
			std::shared_ptr<com::rpc::response> response(new com::rpc::response());

			response->set_code((int)code);
			response->set_rpc_id(request->rpc_id());
			if (!this->Send(address, response))
			{

			}
		}
	}

	void RpcClientComponent::OnResponse(std::shared_ptr<com::rpc::response> response)
	{
        long long taskId = response->rpc_id();
        assert(this->GetApp()->IsMainThread());
        this->mRpcComponent->OnResponse(taskId, response);
	}

	std::shared_ptr<MessageRpcClient> RpcClientComponent::GetOrCreateSession(const std::string& address)
	{
		std::shared_ptr<MessageRpcClient> localSession = this->GetSession(address);
		if (localSession != nullptr)
		{
			return localSession;
		}
		std::string ip;
		unsigned short port = 0;
		assert(Helper::String::ParseIpAddress(address, ip, port));
		std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(ip, port);
		localSession = make_shared<MessageRpcClient>(this, socketProxy);

		this->mRpcClientMap.emplace(socketProxy->GetAddress(), localSession);
		return localSession;
	}

	std::shared_ptr<MessageRpcClient> RpcClientComponent::GetSession(const std::string& address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			return nullptr;
		}
		return iter->second;
	}


	bool RpcClientComponent::Send(const std::string & address, std::shared_ptr<com::rpc::request> message)
	{
		auto clientSession = this->GetOrCreateSession(address);
		if (message == nullptr || clientSession == nullptr)
		{
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}

	bool RpcClientComponent::Send(const std::string & address, std::shared_ptr<com::rpc::response> message)
	{
		std::shared_ptr<MessageRpcClient> clientSession = this->GetSession(address);
		if (clientSession == nullptr || message == nullptr)
		{
			LOG_ERROR("send message to [" << address << "] failure");
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}
}// namespace Sentry