#include <XCode/XCode.h>

namespace Sentry
{
    enum NetRpcType // 0-255
    {
        None,
        RpcCallType,   //有actionid 有rpcid
        RpcNoticeType, //有actionid 无rpcid
        RpcBackType,   //无actionid 有rpcid

        RpcClientCallType,   //有 actionid 有rpcid 有userid
        RpcClientNoticeType, //有 actionid 无rpcid 有userid
        RpcClientBackType, //无actionid, 有rpcid, 有userid
    };
}

namespace Sentry
{
    struct NetRpcCall
    {
        unsigned short actionId;
        long long rpcId;
        const char * messageData;
    };
}


namespace Sentry
{
    class IMessage
    {
    public:
        long long UserId;
        long long RpcId;

    public:
        virtual bool IsRequestMsg() = 0;
    };

    class RequestMessage : public IMessage
    {
    public:
        std::string Method;
        std::string Service;
        std::string MessageData;
        std::string ProtocolName;

    public:
        bool IsRequestMsg() final { return true; }

    public:
        RequestMessage()
        {
            this->UserId = 0;
            this->RpcId = 0;
        }
    };

    class ResponseMessage : public IMessage
    {
    public:
        XCode Code;
        std::string MessageData;
        std::string ProtocolName;

    public:
        bool IsRequestMsg() final { return false; }

    public:
        ResponseMessage()
        {
            this->UserId = 0;
            this->RpcId = 0;
            this->Code = XCode::Failure;
        }
    };
}