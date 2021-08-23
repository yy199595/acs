#include "NetMessageProxy.h"
#include <Manager/ProtocolManager.h>

namespace Sentry
{
    template<typename T>
    bool MemoryCopy(T *tar, const char *sou, size_t &offset, const size_t maxsize)
    {
        memcpy(tar, sou + offset, sizeof(T));
        offset += sizeof(T);
        return offset <= maxsize;
    }


    NetMessageProxy::NetMessageProxy(NetMessageType type)
            : mMsgType(type)
    {
        this->Clear();
    }

    NetMessageProxy::~NetMessageProxy()
    {
        this->Clear();
    }

    void NetMessageProxy::Clear()
    {
        this->mRpcId = 0;
        this->mUserId = 0;
        this->mProConfig = nullptr;
    }

    NetMessageProxy *NetMessageProxy::Create(const char *message, const size_t size)
    {
        if (message == nullptr || size == 0)
        {
            return nullptr;
        }
        size_t offset = 1;
        Applocation *app = Applocation::Get();
        ProtocolManager *pProtocolMgr = app->GetManager<ProtocolManager>();

        int code = 0;
        long long rpcid = 0;
        long long userid = 0;
        unsigned short actionid = 0;     
        NetMessageType messageType = (NetMessageType) message[0];
        SayNoAssertRetNull_F(MemoryCopy(&actionid, message, offset, size));
		const ProtocolConfig * config = pProtocolMgr->GetProtocolConfig(actionid);
        SayNoAssertRetNull_F(config);

        switch (messageType)
        {
            case S2S_REQUEST:
            SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                break;
            case S2S_RESPONSE:
            SayNoAssertRetNull_F(MemoryCopy(&code, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                break;
            case S2S_NOTICE:

                break;
            case C2S_REQUEST:
            SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
                break;
            case C2S_RESPONSE:
            SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&code, message, offset, size));
                break;
            case C2S_NOTICE:
            case S2C_REQUEST:
            SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
            default:
                return nullptr;
        }

        NetMessageProxy *messageData = new NetMessageProxy(messageType);

        messageData->mRpcId = rpcid;
        messageData->mUserId = userid;
        messageData->mCode = (XCode) code;
        messageData->mProConfig = config;
        if (offset < size)
        {
            const char *msg = message + offset;
            messageData->mMessageData.append(msg, size - offset);
        }
        return messageData;
    }

    NetMessageProxy *NetMessageProxy::Create(NetMessageType type, const std::string &service, const std::string &method)
    {
        if (type > REQUEST_END)
        {
            return nullptr;
        }
        Applocation *app = Applocation::Get();
        ProtocolManager *pProtocolMgr = app->GetManager<ProtocolManager>();

        const ProtocolConfig *config = pProtocolMgr->GetProtocolConfig(service, method);
        if (config != nullptr)
        {
            NetMessageProxy *messageData = new NetMessageProxy(type);
            messageData->mProConfig = config;
            return messageData;
        }
        return nullptr;
    }

	void NetMessageProxy::SetType(NetMessageType type)
	{
		this->mMsgType = type;
		this->mMessageData.clear();
	}


    size_t NetMessageProxy::WriteToBuffer(char *buffer, const size_t size)
    {
        if (this->mMsgType >= REQUEST_END)
        {
            return 0;
        }
		size_t offset = sizeof(unsigned int);
		unsigned char type = (unsigned char)this->mMsgType;  
		memcpy(buffer + offset, &type, sizeof(type)); offset += sizeof(type);
		memcpy(buffer + offset, &mProConfig->MethodId, sizeof(this->mProConfig->MethodId));

		offset += sizeof(this->mProConfig->MethodId);

        if (this->mRpcId != 0)
        {          	
			memcpy(buffer + offset, &mRpcId, sizeof(mRpcId)); offset += sizeof(mRpcId);
        }
        if (this->mUserId != 0)
        {
			memcpy(buffer + offset, &mUserId, sizeof(mUserId)); offset += sizeof(mUserId);
        }	
        memcpy(buffer + offset, this->mMessageData.c_str(), this->mMessageData.size());
        offset += this->mMessageData.size();
        memcpy(buffer, &offset, sizeof(unsigned int));
        return offset;
    }
}
