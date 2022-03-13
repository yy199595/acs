
#include"RpcClientComponent.h"
#include"Object/App.h"
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include<Scene/ThreadPoolComponent.h>
#include"Network/SocketProxy.h"
#include<Rpc/RpcComponent.h>
#ifdef __DEBUG__
#include<Pool/MessagePool.h>
#include"Async/RpcTask/RpcTaskSource.h"
#endif
namespace Sentry
{
    bool RpcClientComponent::Awake()
    {
        this->mRpcComponent = nullptr;
        this->mTaskComponent = nullptr;
        this->mProtoConfigComponent = nullptr;
        return true;
    }
    bool RpcClientComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<RpcComponent>());
        LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<ThreadPoolComponent>());
        LOG_CHECK_RET_FALSE(this->mProtoConfigComponent = this->GetComponent<RpcConfigComponent>());
        return true;
    }

	void RpcClientComponent::OnCloseSocket(long long socketId, XCode code)
	{
		auto iter = this->mRpcClientMap.find(socketId);
		if (iter != this->mRpcClientMap.end())
        {
            std::shared_ptr<ProtoRpcClient> session = iter->second;
            auto iter1 = this->mAddressClientMap.find(session->GetAddress());
            if(iter1 != this->mAddressClientMap.end())
            {
                this->mAddressClientMap.erase(iter1);
            }
#ifdef __DEBUG__
            auto component = App::Get().GetComponent<RpcConfigComponent>();
            LOG_ERROR(session->GetAddress(), " connected code =", component->GetCodeDesc(code));
#endif
            this->mRpcClientMap.erase(iter);
        }
	}

	void RpcClientComponent::OnConnectAfter(long long id, XCode code)
	{
        auto iter = this->mRpcClientMap.find(id);
        LOG_CHECK_RET(iter != this->mRpcClientMap.end());

        auto rpcClient = iter->second;
		const std::string & address = rpcClient->GetAddress();
		if (code != XCode::Successful)
        {
            LOG_ERROR("connect ", address, " failure");
        }
		else
        {
            rpcClient->StartReceive();
            const std::string &name = rpcClient->GetSocketProxy()->GetName();
            LOG_INFO("[", name, " => ", address, "] start receive message");
        }
	}

	void RpcClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        // 判断是不是服务器根据 白名单
        long long id = socket->GetSocketId();
        auto iter = this->mRpcClientMap.find(id);
        if(iter == this->mRpcClientMap.end())
        {
            std::shared_ptr<ProtoRpcClient> tcpSession(
                    new ProtoRpcClient(this, socket, SocketType::RemoteSocket));

            tcpSession->StartReceive();
            this->mRpcClientMap.emplace(id, tcpSession);
        }
    }

    void RpcClientComponent::StartClose(long long id)
    {
        auto iter = this->mRpcClientMap.find(id);
        if(iter != this->mRpcClientMap.end())
        {
            auto rpcClient = iter->second;
            if(rpcClient->IsOpen())
            {
                rpcClient->StartClose();
            }
            this->mRpcClientMap.erase(iter);
        }
    }

    void RpcClientComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
    {
        long long socketId = request->socket_id();
        XCode code = this->mRpcComponent->OnRequest(request);
        if(code != XCode::Successful)
        {
            std::shared_ptr<com::Rpc_Response> response(new com::Rpc_Response());

            response->set_code((int)code);
            response->set_rpc_id(request->rpc_id());
            response->set_user_id(request->user_id());
            if(!this->Send(socketId, response))
            {
                this->OnSendFailure(socketId, response);
            }
        }
    }

    void RpcClientComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
		this->mRpcComponent->OnResponse(response);
	}

    std::shared_ptr<ProtoRpcClient> RpcClientComponent::MakeSession(const std::string &name, const std::string & address)
	{
        auto localSession = this->GetSession(address);
        if(localSession != nullptr)
        {
            return localSession;
        }
#ifdef ONLY_MAIN_THREAD
        IAsioThread & workThread = App::Get().GetTaskScheduler();
#else
        IAsioThread & workThread = this->mTaskComponent->AllocateNetThread();
#endif
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, name));
        localSession = make_shared<ProtoRpcClient>(this, socketProxy, SocketType::LocalSocket);

        this->mAddressClientMap.emplace(address, socketProxy->GetSocketId());
		this->mRpcClientMap.emplace(socketProxy->GetSocketId(), localSession);
		return localSession;
	}

    void RpcClientComponent::OnDestory()
    {
    }

    std::shared_ptr<ProtoRpcClient> RpcClientComponent::GetSession(long long id)
    {
        auto iter = this->mRpcClientMap.find(id);
        return iter != this->mRpcClientMap.end() ? iter->second : nullptr;
    }

    std::shared_ptr<ProtoRpcClient> RpcClientComponent::GetSession(const std::string &address)
    {
        auto iter = this->mAddressClientMap.find(address);
        if(iter == this->mAddressClientMap.end())
        {
            return nullptr;
        }
        return this->GetSession(iter->second);
    }

    bool RpcClientComponent::CloseSession(long long id)
    {
        auto iter = this->mRpcClientMap.find(id);
        if (iter != this->mRpcClientMap.end())
        {
			iter->second->StartClose();           
            return true;
        }
        return false;
    }

	bool RpcClientComponent::Send(long long id, std::shared_ptr<com::Rpc_Request> message)
    {
         auto clientSession = this->GetSession(id);
        if (message == nullptr || clientSession == nullptr)
        {
            return false;
        }
#ifdef __DEBUG__
		std::string json;
        auto config = App::Get().GetComponent<RpcConfigComponent>()->
                GetProtocolConfig(message->method_id());
        LOG_DEBUG("=============== [send request] ===============");
        LOG_DEBUG("func = ", config->Service,'.', config->Method);
		if(Helper::Proto::GetJson(message, json))
		{
			LOG_DEBUG("json = ", json);
		}
        LOG_DEBUG("==============================================");
#endif
        clientSession->SendToServer(message);
        return true;
    }

	bool RpcClientComponent::Send(long long id, std::shared_ptr<com::Rpc_Response> message)
    {
        auto clientSession = this->GetSession(id);
        if(clientSession == nullptr || message == nullptr)
        {
            return false;
        }
        clientSession->SendToServer(message);
        return true;
    }
}// namespace Sentry