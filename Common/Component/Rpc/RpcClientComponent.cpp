
#include"RpcClientComponent.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Network/SocketProxy.h"
#include"Component/Rpc/ServiceRpcComponent.h"
#include"Global/ServiceConfig.h"
#include"Component/Scene/NetThreadComponent.h"

#ifdef __DEBUG__
#include"Async/RpcTask/RpcTaskSource.h"
#include<google/protobuf/util/json_util.h>
#include"Network/Listener/TcpServerComponent.h"
#endif
namespace Sentry
{
	void RpcClientComponent::Awake()
	{
		this->mRpcComponent = nullptr;
        this->mTcpComponent = nullptr;
	}
	bool RpcClientComponent::LateAwake()
	{
        LOG_CHECK_RET_FALSE(this->mTcpComponent = this->GetComponent<TcpServerComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<ServiceRpcComponent>());
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
			std::shared_ptr<ServerClientContext> tcpSession
					= std::make_shared<ServerClientContext>(this, socket);

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

	void RpcClientComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
	{
        assert(this->GetApp()->IsMainThread());
        const std::string & address = request->address();
		XCode code = this->mRpcComponent->OnRequest(request);
		if (code != XCode::Successful)
		{
			std::shared_ptr<com::Rpc_Response> response(new com::Rpc_Response());

			response->set_code((int)code);
			response->set_rpc_id(request->rpc_id());
			response->set_user_id(request->user_id());
			if (!this->Send(address, response))
			{

			}
		}
	}

	void RpcClientComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
        long long taskId = response->rpc_id();
        assert(this->GetApp()->IsMainThread());
        this->mRpcComponent->OnResponse(taskId, response);
	}

	std::shared_ptr<ServerClientContext> RpcClientComponent::GetOrCreateSession(const std::string& address)
	{
		std::shared_ptr<ServerClientContext> localSession = this->GetSession(address);
		if (localSession != nullptr)
		{
			return localSession;
		}
		std::string ip;
		unsigned short port = 0;
		assert(Helper::String::ParseIpAddress(address, ip, port));
		std::shared_ptr<SocketProxy> socketProxy = this->mTcpComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(ip, port);
		localSession = make_shared<ServerClientContext>(this, socketProxy);

		this->mRpcClientMap.emplace(socketProxy->GetAddress(), localSession);
		return localSession;
	}

	std::shared_ptr<ServerClientContext> RpcClientComponent::GetSession(const std::string& address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			return nullptr;
		}
		return iter->second;
	}


	bool RpcClientComponent::Send(const std::string & address, std::shared_ptr<com::Rpc_Request> message)
	{
		auto clientSession = this->GetOrCreateSession(address);
		if (message == nullptr || clientSession == nullptr)
		{
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}

	bool RpcClientComponent::Send(const std::string & address, std::shared_ptr<com::Rpc_Response> message)
	{
		std::shared_ptr<ServerClientContext> clientSession = this->GetSession(address);
		if (clientSession == nullptr || message == nullptr)
		{
			LOG_ERROR("send message to [" << address << "] failure");
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}
}// namespace Sentry