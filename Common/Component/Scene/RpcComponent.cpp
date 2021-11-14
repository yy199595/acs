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

	void RpcComponent::OnCloseSession(RpcClient * socket, XCode code)
	{
		if (socket == nullptr)
		{
			return;
		}
		const std::string & address = socket->GetAddress();
		long long id = socket->GetSocketProxy().GetSocketId();
		auto iter = this->mSessionAdressMap.find(id);
		if (iter != this->mSessionAdressMap.end())
		{
			this->mSessionAdressMap.erase(iter);
			GKDebugError("remove tcp socket " << address);
			delete socket;
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
        std::string json;
		const std::string & method = config->Method;
		const std::string & service = config->Service;
		if (util::MessageToJsonString(*request, &json).ok())
		{
			GKDebugLog("[" << session->GetAddress() << " request] "
				<< service << "." << method << "  json = \n" << StringHelper::FormatJson(json));
		}
#endif
        this->mRequestComponent->OnRequest(*request);
    }

    void RpcComponent::OnResponse(RpcClient *session, com::Rpc_Response *response)
    {
#ifdef __DEBUG__
        std::string json;
        if(util::MessageToJsonString(*response, &json).ok())
        {			
			long long rpcId = response->rpcid();
			auto rpc = this->mResponseComponent->GetRpcHandler(rpcId);
			if (rpc != nullptr)
			{
				auto config = this->mProtocolComponent->GetProtocolConfig(rpc->GetMethodId());
				if (config != nullptr)
				{					
					const std::string & method = config->Method;
					const std::string & service = config->Service;
					long long nowTime = TimeHelper::GetMilTimestamp();
					float time = (nowTime - rpc->GetCreateTime()) / 1000.0f;
					GKDebugLog("[" << session->GetAddress() << " response]  [time = " 
						<< time <<"]  "<< service << "." << method <<  " json = \n" << StringHelper::FormatJson(json));
				}
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

    RpcClient *RpcComponent::GetRemoteSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        return iter == this->mSessionAdressMap.end() ? nullptr : iter->second;
    }

    long long RpcComponent::NewSession(const std::string &name, const std::string &ip,
                                                     unsigned short port)
	{
		NetWorkThread &  nThread = mTaskComponent->GetNetThread();
		auto localSession = new RpcConnector(this, ip, port);
        localSession->SetSocket(new SocketProxy(nThread, name));
		this->mSessionAdressMap.emplace(localSession->GetSocketProxy().GetSocketId(), localSession);
		return localSession->GetSocketProxy().GetSocketId();
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