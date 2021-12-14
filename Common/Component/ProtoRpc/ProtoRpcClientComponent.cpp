
#include"ProtoRpcClientComponent.h"
#include<Core/App.h>
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include<Scene/ThreadPoolComponent.h>
#include"Network/SocketProxy.h"
#include<ProtoRpc/ProtoRpcComponent.h>
#ifdef __DEBUG__
#include<Pool/MessagePool.h>
#include<Async/RpcTask/ProtoRpcTask.h>
#endif
namespace GameKeeper
{
    bool ProtoRpcClientComponent::Awake()
    {
        this->mRpcComponent = nullptr;
        this->mTaskComponent = nullptr;
        this->mProtoConfigComponent = nullptr;
        return true;
    }
    bool ProtoRpcClientComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mRpcComponent = this->GetComponent<ProtoRpcComponent>());
        LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<ThreadPoolComponent>());
        LOG_CHECK_RET_FALSE(this->mProtoConfigComponent = this->GetComponent<RpcConfigComponent>());
        return true;
    }

	void ProtoRpcClientComponent::OnCloseSocket(long long socketId, XCode code)
	{
		auto iter = this->mSessionAdressMap.find(socketId);
		if (iter != this->mSessionAdressMap.end())
		{
			ProtoRpcClient * client = iter->second;
			this->mSessionAdressMap.erase(iter);
			LOG_ERROR("remove tcp socket " << client->GetAddress());
			delete client;
		}		
	}

	void ProtoRpcClientComponent::OnConnectAfter(long long id, XCode code)
	{
        auto iter = this->mSessionAdressMap.find(id);
        LOG_CHECK_RET(iter != this->mSessionAdressMap.end());

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

	void ProtoRpcClientComponent::OnListen(SocketProxy * socket)
	{
        // 判断是不是服务器根据 白名单
		long long id = socket->GetSocketId();
		auto iter = this->mSessionAdressMap.find(id);
		LOG_CHECK_RET(iter == this->mSessionAdressMap.end());
		auto tcpSession = this->mClientPool.New(this, socket, SocketType::RemoteSocket);
        if(tcpSession != nullptr)
        {
            tcpSession->StartReceive();
            this->mSessionAdressMap.emplace(id, tcpSession);
        }
	}

    void ProtoRpcClientComponent::StartClose(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        if(iter != this->mSessionAdressMap.end())
        {
            ProtoRpcClient * rpcClient = iter->second;
            if(rpcClient->IsOpen())
            {
                rpcClient->StartClose();
            }
        }
    }

    void ProtoRpcClientComponent::OnRequest(com::Rpc_Request *request)
    {
#ifdef __DEBUG__
        auto config = this->mProtoConfigComponent->GetProtocolConfig(request->methodid());
        LOG_INFO("*****************[receive request]******************");
        LOG_DEBUG("func = " << config->Service << "." << config->Method);
        if(request->has_data())
        {
            std::string json;
            if(util::MessageToJsonString(request->data(), &json).ok())
            {
                LOG_DEBUG("json = " << json);
            }
        }
        LOG_INFO("*********************************************");
#endif
        long long socketId = request->socketid();
        if(!this->mRpcComponent->OnRequest(request))
        {
            delete request;
            this->StartClose(socketId);
        }
    }

    void ProtoRpcClientComponent::OnResponse(com::Rpc_Response *response)
    {
#ifdef __DEBUG__
        long long rpcId = response->rpcid();
        auto rpcTask = this->mRpcComponent->GetRpcTask(rpcId);
        if (rpcTask != nullptr)
        {
            auto config = this->mProtoConfigComponent->GetProtocolConfig(rpcTask->GetMethodId());
            if (config != nullptr)
            {
                long long t = TimeHelper::GetMilTimestamp() - rpcTask->GetCreateTime();
                LOG_DEBUG("*****************[receive response]******************");
                LOG_DEBUG("func = " << config->Service << "." << config->Method);
                LOG_DEBUG("time = " << (t / 1000.0f) << "s");
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

    ProtoRpcClient *ProtoRpcClientComponent::GetRpcSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        return iter == this->mSessionAdressMap.end() ? nullptr : iter->second;
    }

    ProtoRpcClient * ProtoRpcClientComponent::NewSession(const std::string &name)
	{
        auto socketProxy = new SocketProxy(mTaskComponent->AllocateNetThread(), name);
        auto localSession = this->mClientPool.New(this, socketProxy, SocketType::LocalSocket);
        if(localSession == nullptr)
        {
            delete socketProxy;
            return nullptr;
        }
		this->mSessionAdressMap.emplace(socketProxy->GetSocketId(), localSession);
		return localSession;
	}

    void ProtoRpcClientComponent::OnDestory()
    {
    }

    ProtoRpcClient *ProtoRpcClientComponent::GetSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        return iter != this->mSessionAdressMap.end() ? iter->second : nullptr;
    }

    bool ProtoRpcClientComponent::CloseSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        if (iter != this->mSessionAdressMap.end())
        {
			iter->second->StartClose();           
            return true;
        }
        return false;
    }

	bool ProtoRpcClientComponent::SendByAddress(long long id, com::Rpc_Request * message)
	{
		ProtoRpcClient * clientSession = this->GetSession(id);

        LOG_CHECK_RET_FALSE(clientSession);
        LOG_CHECK_RET_FALSE(clientSession->IsOpen());
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
		clientSession->StartSendProtocol(RPC_TYPE_REQUEST, message);
		return true;
	}

	bool ProtoRpcClientComponent::SendByAddress(long long id, com::Rpc_Response * message)
	{
		ProtoRpcClient * clientSession = this->GetSession(id);

        LOG_CHECK_RET_FALSE(clientSession);
        LOG_CHECK_RET_FALSE(clientSession->IsOpen());
#ifdef __DEBUG__
        std::string json;
        util::MessageToJsonString(*message, &json);
        LOG_DEBUG("=============== [send response] ===============");
        LOG_DEBUG("json = " << json);
        LOG_DEBUG("==============================================");
#endif
		clientSession->StartSendProtocol(RPC_TYPE_RESPONSE, message);
		return true;
	}
}// namespace GameKeeper