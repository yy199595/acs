#include "Component/Scene/RpcComponent.h"

#include <Core/App.h>
#include <Scene/RpcResponseComponent.h>
#include <Util/StringHelper.h>
#include <Scene/RpcProtoComponent.h>
#include <Scene/TaskPoolComponent.h>
#include <Component/Scene/RpcRequestComponent.h>
#include <Network/Rpc/RpcConnector.h>
#ifdef __DEBUG__
#include<Pool/MessagePool.h>
#include<Method/CallHandler.h>
#endif
namespace GameKeeper
{
    bool RpcComponent::Awake()
    {

		GKAssertRetFalse_F(this->mTaskComponent = this->GetComponent<TaskPoolComponent>());
		GKAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<RpcProtoComponent>());
		GKAssertRetFalse_F(this->mRequestComponent = this->GetComponent<RpcRequestComponent>());
        GKAssertRetFalse_F(this->mResponseComponent = this->GetComponent<RpcResponseComponent>());

        return true;
    }

	void RpcComponent::OnCloseSession(long long socketId, XCode code)
	{
		auto iter = this->mSessionAdressMap.find(socketId);
		if (iter != this->mSessionAdressMap.end())
		{
			RpcClient * client = iter->second;
			this->mSessionAdressMap.erase(iter);
			GKDebugError("remove tcp socket " << client->GetAddress());
			delete client;
		}		
	}

    void RpcComponent::OnSendMessageAfter(RpcClient *session, std::string * message, XCode code)
    {
#ifdef __DEBUG__
        const std::string & address = session->GetAddress();
        auto type = (DataMessageType)message->at(sizeof(unsigned int));
        if(type == DataMessageType::TYPE_REQUEST)
        {
            GKDebugWarning(address << " send request message successful size = " << message->size());
        }
        else if(type == DataMessageType::TYPE_RESPONSE)
        {
            GKDebugWarning(address << " send response message successful size = " << message->size());
        }
#endif
		GStringPool.Destory(message);
    }

	void RpcComponent::OnConnectRemoteAfter(RpcConnector *session, XCode code)
	{
		const std::string & address = session->GetAddress();
		if (code != XCode::Successful)
		{
			GKDebugError("connect to " << address << " failure ");
		}
		else
		{
			long long id = session->GetSocketProxy().GetSocketId();
			const std::string & name = session->GetSocketProxy().GetName();
			GKDebugInfo(
				"connect to [" << name << ":" << address << "] successful");
			this->mSessionAdressMap.emplace(id, session);
		}
	}

	void RpcComponent::OnListen(SocketProxy * socket)
	{
		long long id = socket->GetSocketId();
		auto iter = this->mSessionAdressMap.find(id);
		if (iter == this->mSessionAdressMap.end())
        {
            auto tcpSession = new RpcClient(this);
            tcpSession->SetSocket(socket);
            this->mSessionAdressMap.emplace(id, tcpSession);
        }
	}

    void RpcComponent::OnRequest(RpcClient *session, com::Rpc_Request *request)
    {
        const int methodId = request->methodid();
        const ProtocolConfig * config = this->mProtocolComponent->GetProtocolConfig(methodId);
        if(config == nullptr)
        {
			delete request;
            session->StartClose();
            return;
        }
#ifdef __DEBUG__
        GKDebugInfo("*****************[request]******************");
        GKDebugLog("func = " << config->Service << "." << config->Method);
        if(request->has_data())
        {
            std::string json;
            if(util::MessageToJsonString(request->data(), &json).ok())
            {
                GKDebugLog("json = " << StringHelper::FormatJson(json));
            }
        }
        GKDebugInfo("*********************************************");
#endif
        this->mRequestComponent->OnRequest(*request);
    }

    void RpcComponent::OnResponse(RpcClient *session, com::Rpc_Response *response)
    {
#ifdef __DEBUG__
			long long rpcId = response->rpcid();
			auto rpc = this->mResponseComponent->GetRpcHandler(rpcId);
			if (rpc != nullptr)
            {
                auto config = this->mProtocolComponent->GetProtocolConfig(rpc->GetMethodId());
                if (config != nullptr)
                {
                    long long nowTime = TimeHelper::GetMilTimestamp();
                    float time = (nowTime - rpc->GetCreateTime()) / 1000.0f;

                    GKDebugLog("*****************[response]******************");
                    GKDebugLog("func = " << config->Service << "." << config->Method);
                    GKDebugLog("time = " << time << "s");
                    if ((XCode) response->code() != XCode::Successful)
                    {
                        auto codeConfig = this->mProtocolComponent->GetCodeConfig(response->code());
                        GKDebugError("code = " << codeConfig->Name << ":" << codeConfig->Desc);
                    }
                    else if (response->has_data())
                    {
                        std::string json;
                        if (util::MessageToJsonString(response->data(), &json).ok())
                        {
                            GKDebugLog("json = \n" << StringHelper::FormatJson(json));
                        }
                    }
                    GKDebugLog("*********************************************");
                }
            }
#endif
        this->mResponseComponent->OnResponse(*response);
		delete response;
    }

    RpcConnector *RpcComponent::GetLocalSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        if(iter == this->mSessionAdressMap.end())
        {
            return nullptr;
        }
        RpcClient * session = iter->second;
        if(session->GetSocketType() == SocketType::RemoteSocket)
        {
            return nullptr;
        }
        return static_cast<RpcConnector*>(session);
    }

    RpcClient *RpcComponent::GetRpcSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        return iter == this->mSessionAdressMap.end() ? nullptr : iter->second;
    }

     RpcConnector * RpcComponent::NewSession(const std::string &name, const std::string &ip,
                                                     unsigned short port)
	{
		NetWorkThread &  nThread = mTaskComponent->AllocateNetThread();

        auto * socketProxy = new SocketProxy(nThread, name);
		auto localSession = new RpcConnector(this, ip, port);
        localSession->SetSocket(socketProxy);
		this->mSessionAdressMap.emplace(socketProxy->GetSocketId(), localSession);
		return localSession;
	}

    void RpcComponent::OnDestory()
    {
    }

    RpcClient *RpcComponent::GetSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        return iter != this->mSessionAdressMap.end() ? iter->second : nullptr;
    }

    bool RpcComponent::CloseSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        if (iter != this->mSessionAdressMap.end())
        {
			iter->second->StartClose();           
            return true;
        }
        return false;
    }

	bool RpcComponent::SendByAddress(long long id, com::Rpc_Request * message)
	{
		RpcClient * clientSession = this->GetSession(id);
		if (clientSession == nullptr || !clientSession->IsOpen())
		{
			return false;
		}
		clientSession->StartSendProtocol(RPC_TYPE_REQUEST, message);
		return true;
	}

	bool RpcComponent::SendByAddress(long long id, com::Rpc_Response * message)
	{
		RpcClient * clientSession = this->GetSession(id);
		if (clientSession == nullptr || !clientSession->IsOpen())
		{
			return false;
		}
		clientSession->StartSendProtocol(RPC_TYPE_RESPONSE, message);
		return true;
	}

}// namespace GameKeeper