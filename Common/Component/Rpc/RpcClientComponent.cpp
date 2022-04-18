
#include"RpcClientComponent.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Network/SocketProxy.h"
#include"Component/Rpc/RpcComponent.h"
#include"Global/RpcConfig.h"
#include"Component/Scene/ThreadPoolComponent.h"

#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#include"Async/RpcTask/RpcTaskSource.h"
#endif
namespace Sentry
{
	void RpcClientComponent::Awake()
	{
		this->mRpcComponent = nullptr;
		this->mTaskComponent = nullptr;
	}
	bool RpcClientComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<ThreadPoolComponent>());
		return true;
	}

	void RpcClientComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			this->mRpcClientMap.erase(iter);
#ifdef __DEBUG__
			const RpcConfig & rpcConfig = App::Get()->GetRpcConfig();
			LOG_ERROR(address << " connected code " << rpcConfig.GetCodeDesc(code));
#endif
		}
	}

	void RpcClientComponent::OnConnectAfter(const std::string & address, XCode code)
	{
		assert(!address.empty());
		auto iter = this->mRpcClientMap.find(address);
		LOG_CHECK_RET(iter != this->mRpcClientMap.end());

		auto rpcClient = iter->second;
		if (code == XCode::Successful)
		{
			rpcClient->StartReceive();
		}
	}

	void RpcClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		// 判断是不是服务器根据 白名单
		const std::string & address = socket->GetAddress();
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			std::shared_ptr<ProtoRpcClient> tcpSession(
				new ProtoRpcClient(this, socket, SocketType::RemoteSocket));

			tcpSession->StartReceive();
			assert(!address.empty());
			this->mRpcClientMap.emplace(address, tcpSession);
		}
	}

	void RpcClientComponent::StartClose(const std::string & address)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			auto rpcClient = iter->second;
			if (rpcClient->IsOpen())
			{
				rpcClient->StartClose();
			}
			this->mRpcClientMap.erase(iter);
		}
	}

	void RpcClientComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
	{
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
				this->OnSendFailure(address, response);
			}
		}
	}

	void RpcClientComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
		this->mRpcComponent->OnResponse(response);
	}

	std::shared_ptr<ProtoRpcClient> RpcClientComponent::GetOrCreateSession(const std::string& name, const std::string& address)
	{
		auto localSession = this->GetSession(address);
		if (localSession != nullptr)
		{
			return localSession;
		}
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		IAsioThread & workThread = this->mTaskComponent->AllocateNetThread();
#endif
		std::string ip;
		unsigned short port = 0;
		if(!Helper::String::ParseIpAddress(address, ip, port))
		{
			return nullptr;
		}
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, ip, port));
		localSession = make_shared<ProtoRpcClient>(this, socketProxy, SocketType::LocalSocket);

		this->mRpcClientMap.emplace(socketProxy->GetAddress(), localSession);
		return localSession;
	}

	std::shared_ptr<ProtoRpcClient> RpcClientComponent::GetSession(const std::string& address)
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
		auto clientSession = this->GetSession(address);
		if (message == nullptr || clientSession == nullptr)
		{
			return false;
		}
#ifdef __DEBUG__
		std::string json;
		auto config = App::Get()->GetRpcConfig().
			GetProtocolConfig(message->method_id());
		LOG_DEBUG("=============== [send request] ===============");
		LOG_DEBUG("func = " << config->Service << "."<< config->Method);
		if (Helper::Proto::GetJson(message, json))
		{
			LOG_DEBUG("json = " << json);
		}
		LOG_DEBUG("==============================================");
#endif
		clientSession->SendToServer(message);
		return true;
	}

	bool RpcClientComponent::Send(const std::string & address, std::shared_ptr<com::Rpc_Response> message)
	{
		auto clientSession = this->GetSession(address);
		if (clientSession == nullptr || message == nullptr)
		{
			LOG_ERROR("send message to [" << address << "] failure");
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}
}// namespace Sentry