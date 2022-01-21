#pragma once

#include"ProxyClient.h"
#include"Protocol/s2s.pb.h"
#include"Async/RpcTask/RpcTaskSource.h"

using namespace google::protobuf;
namespace Sentry
{
    class NodeHelper;
    class ServiceProxy
    {
    public:
        explicit ServiceProxy(const std::string &name);

    public:
        void AddAddress(const std::string &address);

        bool RemoveAddress(const std::string &address);

        bool AllotServiceAddress(std::string &address);

        bool RemoveServiceNode(const std::string & address);

        std::shared_ptr<ProxyClient> GetNode(const std::string & address);

        std::shared_ptr<com::Rpc_Request> NewRequest(const std::string &method);

    public:
        XCode Call(const std::string &func, std::shared_ptr<RpcTaskSource> taskSource = nullptr);

        XCode Call(const std::string &func, const Message &message,
                   std::shared_ptr<RpcTaskSource> taskSource = nullptr);

        XCode Call(const std::string &address, const std::string &func,
                   std::shared_ptr<RpcTaskSource> taskSource = nullptr);

        XCode Call(const std::string &address, const std::string &func,
                   const Message &message, std::shared_ptr<RpcTaskSource> taskSource = nullptr);

    private:
        void Destory();
    private:
        std::string mServiceName;
        TaskComponent * mTaskComponent;
        class RpcComponent *mRpcComponent;
        std::queue<std::string> mAllAddress;
        class RedisComponent * mRedisComponent;
        class RpcConfigComponent *mRpcConfigComponent;
        std::unordered_map<std::string, std::shared_ptr<ProxyClient>> mServiceNodeMap;
    };
}// namespace Sentry