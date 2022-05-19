
#include"RpcClientComponent.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Network/SocketProxy.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#include"Global/ServiceConfig.h"
#include"Component/Scene/NetThreadComponent.h"

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
		LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcHandlerComponent>());
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<NetThreadComponent>());
		return true;
	}

	void RpcClientComponent::OnCloseSocket(const std::string & address, XCode code)
	{
		auto iter = this->mRpcClientMap.find(address);
		if (iter != this->mRpcClientMap.end())
		{
			this->mRpcClientMap.erase(iter);
#ifdef __DEBUG__
			const ServiceConfig & rpcConfig = App::Get()->GetServiceConfig();
			LOG_ERROR(address << " connected code " << rpcConfig.GetCodeDesc(code));
#endif
		}
	}

	void RpcClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		// 判断是不是服务器根据 白名单
		const std::string& address = socket->GetAddress();
		auto iter = this->mRpcClientMap.find(address);
		if (iter == this->mRpcClientMap.end())
		{
			assert(!address.empty());
			std::shared_ptr<ServerRpcClientContext> tcpSession
					= std::make_shared<ServerRpcClientContext>(this, socket);

			tcpSession->StartReceive();
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

			}
		}
	}

	void RpcClientComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
		this->mRpcComponent->OnResponse(response);
	}

	std::shared_ptr<ServerRpcClientContext> RpcClientComponent::GetOrCreateSession(const std::string& address)
	{
		std::shared_ptr<ServerRpcClientContext> localSession = this->GetSession(address);
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
		assert(Helper::String::ParseIpAddress(address, ip, port));
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, ip, port));
		localSession = make_shared<ServerRpcClientContext>(this, socketProxy);

		this->mRpcClientMap.emplace(socketProxy->GetAddress(), localSession);
		return localSession;
	}

	std::shared_ptr<ServerRpcClientContext> RpcClientComponent::GetSession(const std::string& address)
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
		auto config = App::Get()->GetServiceConfig().
				GetInterfaceConfig(message->method_id());
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
		std::shared_ptr<ServerRpcClientContext> clientSession = this->GetSession(address);
		if (clientSession == nullptr || message == nullptr || !clientSession->IsOpen())
		{
			LOG_ERROR("send message to [" << address << "] failure");
			return false;
		}
		clientSession->SendToServer(message);
		return true;
	}
}// namespace Sentry