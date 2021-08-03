#include "NetMessageProxy.h"
#include <Manager/ProtocolManager.h>

namespace Sentry
{
    template<typename T>
    bool MemoryCopy(T *tar, const char *sou, size_t &offset, const size_t maxsize)
    {
        memcpy((char *) tar, sou + offset, sizeof(T));
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
        const ProtocolConfig *config = nullptr;

        NetMessageType messageType = (NetMessageType) message[0];

        //SayNoAssertRetNull_F(MemoryCopy(actionid, message, offset, size));
        config = pProtocolMgr->GetProtocolConfig(actionid);
        SayNoAssertRetNull_F(config);

        switch (messageType)
        {
            case s2sRequest:
            SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                break;
            case s2sResponse:
            SayNoAssertRetNull_F(MemoryCopy(&code, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                break;
            case s2sNotice:

                break;
            case c2sRequest:
            SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
                break;
            case c2sResponse:
            SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&code, message, offset, size));
                break;
            case c2sNotice:
            case s2cNotice:
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
        if (type > RequestEnd)
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

    bool NetMessageProxy::InitMessageParame(Message *message, long long rcpId, long long userId)
    {
        if (this->mMsgType > RequestEnd)
        {
            return false;
        }
        this->mRpcId = rcpId;
        this->mUserId = userId;
        if (message != nullptr)
        {
            if (!message->SerializePartialToString(&this->mMessageData))
            {
                return false;
            }
        }
        return true;
    }

    bool NetMessageProxy::InitMessageData(XCode code, Message *message)
    {
        if (this->mProConfig == nullptr)
        {
            return false;
        }
        if (this->mMsgType >= RequestEnd)
        {
            return false;
        }
        if (this->mRpcId == 0)
        {
            return false;
        }
        if (this->mUserId != 0)
        {
            SayNoAssertRetFalse_F(this->mMsgType == s2sRequest);
            this->mMsgType = c2sResponse;
        }
        SayNoAssertRetFalse_F(this->mMsgType == s2sRequest);
        if (message != nullptr && code == XCode::Successful)
        {
            if (!message->SerializePartialToString(&this->mMessageData))
            {
                return false;
            }
        }
        this->mCode = code;
        return true;
    }

    bool NetMessageProxy::InitMessageData(Message *message, long long rpcId, long long userId)
    {
        SayNoAssertRetFalse_F(this->mProConfig);
        this->mRpcId = rpcId;
        this->mUserId = userId;
        if (message != nullptr)
        {
            if (!message->SerializePartialToString(&this->mMessageData))
            {
                return false;
            }
        }
        return true;
    }


    size_t NetMessageProxy::WriteToBuffer(char *buffer, const size_t size)
    {
        if (this->mMsgType >= RequestEnd)
        {
            return 0;
        }
        size_t offset = sizeof(unsigned int);
        memcpy(buffer, &this->mProConfig->MethodId, sizeof(this->mProConfig->MethodId));
        offset += sizeof(this->mProConfig->MethodId);
        if (this->mRpcId != 0)
        {
            memcpy(buffer, &this->mRpcId, sizeof(this->mRpcId));
            offset += sizeof(this->mRpcId);
        }
        if (this->mUserId != 0)
        {
            memcpy(buffer, &this->mUserId, sizeof(this->mUserId));
            offset += sizeof(this->mUserId);
        }
        memcpy(buffer + offset, this->mMessageData.c_str(), this->mMessageData.size());
        offset += this->mMessageData.size();
        memcpy(buffer, &offset, sizeof(unsigned int));
        return offset;
    }
}
