
#include"RpcClientComponent.h"
#include<Core/App.h>
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include<Scene/ThreadPoolComponent.h>
#include"Network/SocketProxy.h"
#include<ServerRpc/RpcComponent.h>
#ifdef __DEBUG__
#include<Pool/MessagePool.h>
#include<Async/RpcTask/RpcTask.h>
#endif
namespace GameKeeper
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
            ProtoRpcClient *client = iter->second;
            this->mRpcClientMap.erase(iter);
#ifdef __DEBUG__
            auto component = App::Get().GetComponent<RpcConfigComponent>();
            LOG_ERROR("remove tcp socket " << client->GetAddress() <<
                                           " error :" << component->GetCodeDesc(code));
#endif
            delete client;
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
			LOG_ERROR("connect to " << address << " failure ");
		}
		else
		{
            rpcClient->StartReceive();
			const std::string & name = rpcClient->GetSocketProxy().GetName();
			LOG_INFO("connect to [" << name << ":" << address << "] successful");
		}
	}

	void RpcClientComponent::OnListen(SocketProxy * socket)
	{
        // 判断是不是服务器根据 白名单
		long long id = socket->GetSocketId();
		auto iter = this->mRpcClientMap.find(id);
		LOG_CHECK_RET(iter == this->mRpcClientMap.end());
		auto tcpSession = this->mClientPool.New(this, socket, SocketType::RemoteSocket);
        if(tcpSession != nullptr)
        {
            tcpSession->StartReceive();
            this->mRpcClientMap.emplace(id, tcpSession);
        }
	}

    void RpcClientComponent::StartClose(long long id)
    {
        auto iter = this->mRpcClientMap.find(id);
        if(iter != this->mRpcClientMap.end())
        {
            ProtoRpcClient * rpcClient = iter->second;
            if(rpcClient->IsOpen())
            {
                rpcClient->StartClose();
            }
        }
    }

    void RpcClientComponent::OnRequest(com::Rpc_Request *request)
    {
        long long socketId = request->socketid();
        if(!this->mRpcComponent->OnRequest(request))
        {
            delete request;
            this->StartClose(socketId);
        }
    }

    void RpcClientComponent::OnResponse(com::Rpc_Response *response)
    {
#ifdef __DEBUG__
        long long rpcId = response->rpcid();
        auto rpcTask = this->mRpcComponent->GetRpcTask(rpcId);
        if (rpcTask != nullptr)
        {
            auto config = this->mProtoConfigComponent->GetProtocolConfig(rpcTask->GetMethodId());
            if (config != nullptr)
            {
                LOG_DEBUG("*****************[receive response]******************");
                LOG_DEBUG("func = " << config->Service << "." << config->Method);
                LOG_DEBUG("time = " << rpcTask->GetCostTime() << "ms");
                if ((XCode) response->code() != XCode::Successful)
                {
                    auto codeConfig = this->mProtoConfigComponent->GetCodeConfig(response->code());
                    LOG_ERROR("code = " << codeConfig->Name << ":" << codeConfig->Desc);
                }
                else if (response->has_data())
                {
                    std::string json;
                    if (util::MessageToJsonString(response->data(), &json).ok())
                    {
                        LOG_DEBUG("json = " << json);
                    }
                }
                LOG_DEBUG("*********************************************");
            }
        }
#endif
        this->mRpcComponent->OnResponse(response);
    }

    ProtoRpcClient *RpcClientComponent::GetRpcSession(long long id)
    {
        auto iter = this->mRpcClientMap.find(id);
        return iter == this->mRpcClientMap.end() ? nullptr : iter->second;
    }

    ProtoRpcClient * RpcClientComponent::NewSession(const std::string &name)
	{
        auto socketProxy = new SocketProxy(mTaskComponent->AllocateNetThread(), name);
        auto localSession = this->mClientPool.New(this, socketProxy, SocketType::LocalSocket);
        if(localSession == nullptr)
        {
            delete socketProxy;
            return nullptr;
        }
		this->mRpcClientMap.emplace(socketProxy->GetSocketId(), localSession);
		return localSession;
	}

    void RpcClientComponent::OnDestory()
    {
    }

    ProtoRpcClient *RpcClientComponent::GetSession(long long id)
    {
        auto iter = this->mRpcClientMap.find(id);
        return iter != this->mRpcClientMap.end() ? iter->second : nullptr;
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

	bool RpcClientComponent::SendByAddress(long long id, com::Rpc_Request * message)
	{
		ProtoRpcClient * clientSession = this->GetSession(id);
        if(clientSession == nullptr || !clientSession->IsOpen())
        {
            return false;
        }
#ifdef __DEBUG__
        std::string json;
        util::MessageToJsonString(*message, &json);
        auto config = App::Get().GetComponent<RpcConfigComponent>()->
                GetProtocolConfig(message->methodid());
        LOG_DEBUG("=============== [send request] ===============");
        LOG_DEBUG("func = " << config->Service << "." << config->Method);
        LOG_DEBUG("json = " << json);
        LOG_DEBUG("==============================================");
#endif
        return clientSession->SendToServer(message);
	}

	bool RpcClientComponent::SendByAddress(long long id, com::Rpc_Response * message)
	{
		ProtoRpcClient * clientSession = this->GetSession(id);

        LOG_CHECK_RET_FALSE(clientSession);
#ifdef __DEBUG__
        std::string json;
        util::MessageToJsonString(*message, &json);
        LOG_DEBUG("=============== [send response] ===============");
        LOG_DEBUG("json = " << json);
        LOG_DEBUG("==============================================");
#endif
        return clientSession->SendToServer(message);
	}
}// namespace GameKeeper