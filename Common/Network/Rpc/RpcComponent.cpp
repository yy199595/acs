#include "RpcComponent.h"

#include <Core/App.h>
#include <Scene/RpcResponseComponent.h>
#include <Util/StringHelper.h>
#include <Scene/ProtocolComponent.h>
#include <Scene/TaskPoolComponent.h>
#include <Service/RpcRequestComponent.h>
#include <Network/Rpc/RpcLocalSession.h>
#ifdef __DEBUG__
#include <Pool/MessagePool.h>
#endif
namespace GameKeeper
{
    bool RpcComponent::Awake()
    {

		GKAssertRetFalse_F(this->mTaskComponent = this->GetComponent<TaskPoolComponent>());
		GKAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<ProtocolComponent>());
		GKAssertRetFalse_F(this->mRequestComponent = this->GetComponent<RpcRequestComponent>());
        GKAssertRetFalse_F(this->mResponseComponent = this->GetComponent<RpcResponseComponent>());

        return true;
    }

	void RpcComponent::OnCloseSession(RpcClientSession * socket, XCode code)
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

	void RpcComponent::OnReceiveMessage(RpcClientSession * session, string * message)
	{
		if (!this->OnReceive(session, *message))
		{
			session->StartClose();
		}
		GStringPool.Destory(message);
	}

    void RpcComponent::OnSendMessageAfter(RpcClientSession *session, std::string * message, XCode code)
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

	void RpcComponent::OnConnectRemoteAfter(RpcLocalSession *session, XCode code)
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
            auto tcpSession = new RpcClientSession(this);
            tcpSession->SetSocket(socket);
            this->mSessionAdressMap.emplace(id, tcpSession);
        }
	}


	bool RpcComponent::OnReceive(RpcClientSession *session, const std::string & message)
	{
        const char * body = message.c_str() + 1;
        const size_t bodySize = message.size() - 1;
        const std::string & address = session->GetAddress();
		auto messageType = (DataMessageType)message.at(0);
		if (messageType == DataMessageType::TYPE_REQUEST)
        {
            this->mRequestData.Clear();
            if (!this->mRequestData.ParseFromArray(body, bodySize))
            {
                return false;
            }
            unsigned short methodId = this->mRequestData.methodid();
            auto config = this->mProtocolComponent->GetProtocolConfig(methodId);
            if (config == nullptr)
            {
                return false;
            }
#ifdef __DEBUG__
            std::string json;
            const std::string & data = this->mRequestData.messagedata();
            const std::string method = config->ServiceName + "." + config->Method;
            if(!config->RequestMessage.empty())
            {
               Message * msg = MessagePool::NewByData(config->RequestMessage, data);
               util::MessageToJsonString(*msg, &json);
            }
            GKDebugLog("[request " << method << "] json = " << json);
#endif
            this->mRequestData.set_socketid(session->GetSocketProxy().GetSocketId());
            return this->mRequestComponent->OnRequest(mRequestData);
        }
		else if (messageType == DataMessageType::TYPE_RESPONSE)
		{
            this->mResponseData.Clear();
            if(!this->mResponseData.ParseFromArray(body, bodySize))
            {
                return false;
            }
            unsigned short methodId = this->mResponseData.methodid();
            auto config = this->mProtocolComponent->GetProtocolConfig(methodId);
            if (config == nullptr)
            {
                return false;
            }
#ifdef __DEBUG__
            std::string json;
            const std::string & data = this->mRequestData.messagedata();
            const std::string method = config->ServiceName + "." + config->Method;
            if(!config->ResponseMessage.empty())
            {
                Message * msg = MessagePool::NewByData(config->ResponseMessage, data);
                util::MessageToJsonString(*msg, &json);
            }
            GKDebugLog("[response " << method << "] code:" << this->mResponseData.code() << "  json = " << json);
#endif
            return this->mResponseComponent->OnResponse(mResponseData);
		}
        return false;
	}

    RpcLocalSession *RpcComponent::GetLocalSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        if(iter == this->mSessionAdressMap.end())
        {
            return nullptr;
        }
        RpcClientSession * session = iter->second;
        if(session->GetSocketType() == SocketType::RemoteSocket)
        {
            return nullptr;
        }
        return static_cast<RpcLocalSession*>(session);
    }

    RpcClientSession *RpcComponent::GetRemoteSession(long long id)
    {
        auto iter = this->mSessionAdressMap.find(id);
        return iter == this->mSessionAdressMap.end() ? nullptr : iter->second;
    }

    long long RpcComponent::NewSession(const std::string &name, const std::string &ip,
                                                     unsigned short port)
	{
		NetWorkThread &  nThread = mTaskComponent->GetNetThread();
		auto localSession = new RpcLocalSession(this, ip, port);
        localSession->SetSocket(new SocketProxy(nThread, name));
		this->mSessionAdressMap.emplace(localSession->GetSocketProxy().GetSocketId(), localSession);
		return localSession->GetSocketProxy().GetSocketId();
	}

    void RpcComponent::OnDestory()
    {
    }

    RpcClientSession *RpcComponent::GetSession(long long id)
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

	bool RpcComponent::SendByAddress(long long id, std::string * message)
	{
		RpcClientSession * tcpSession = this->GetSession(id);
		if (tcpSession == nullptr)
		{
			return false;
		}
		tcpSession->StartSendByString(message);
		return true;
	}

	bool RpcComponent::SendByAddress(long long id, com::DataPacket_Request & message)
	{
        std::string * data = this->Serialize(message);
        if(data== nullptr)
        {
            return false;
        }
        this->SendByAddress(id, data);
		return true;
	}

	bool RpcComponent::SendByAddress(long long id, com::DataPacket_Response & message)
	{
		std::string * data = this->Serialize(message);
		if (data == nullptr)
		{
			return false;
		}
		this->SendByAddress(id, data);
		return true;
	}

    std::string *RpcComponent::Serialize(const com::DataPacket_Request &message)
    {
        DataMessageType type = TYPE_REQUEST;
        const size_t size = message.ByteSizeLong() + 5;
        memcpy(this->mMessageBuffer, &size, sizeof(unsigned int));
        memcpy(this->mMessageBuffer + 4, &type, sizeof(char));
        if (!message.SerializeToArray(this->mMessageBuffer + 5, 1024 * 1024 - 5))
        {
            return nullptr;
        }
        return GStringPool.New(this->mMessageBuffer, size);
    }

    std::string *RpcComponent::Serialize(const com::DataPacket_Response &message)
    {
        DataMessageType type = TYPE_RESPONSE;
        const size_t size = message.ByteSizeLong() + 5;
        memcpy(this->mMessageBuffer, &size, sizeof(unsigned int));
        memcpy(this->mMessageBuffer + 4, &type, sizeof(char));
        if (!message.SerializeToArray(this->mMessageBuffer + 5, 1024 * 1024 - 5))
        {
            return nullptr;
        }
        return GStringPool.New(this->mMessageBuffer, size);
    }

}// namespace GameKeeper