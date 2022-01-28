#pragma once

#include"ProxyClient.h"
#include"Protocol/s2s.pb.h"
#include"Async/RpcTask/RpcTaskSource.h"

using namespace google::protobuf;
namespace Sentry
{
    class ServiceProxy
    {
    public:
        explicit ServiceProxy(const std::string &name);

    public:
        std::string AllotAddress(int ms = 5000);

        void AddAddress(const std::string &address);

        bool RemoveAddress(const std::string &address);


        std::shared_ptr<ProxyClient> GetNode(const std::string & address);

        std::shared_ptr<com::Rpc_Request> NewRequest(const std::string &method);

    public:
        std::shared_ptr<RpcTaskSource> Call(const std::string &func);

        std::shared_ptr<RpcTaskSource> Call(const std::string &func, const Message &message);

        std::shared_ptr<RpcTaskSource> Call(const std::string &address, const std::string &func);

        std::shared_ptr<RpcTaskSource> Call(const std::string &address, const std::string &func, const Message &message);

    private:
        void Destory();
    private:
        size_t mIndex;
        class App & mApp;
        std::string mServiceName;
        TaskComponent * mTaskComponent;
        TimerComponent * mTimerComponent;
        class RpcComponent *mRpcComponent;
        std::vector<std::string> mAllAddress;
        class RpcConfigComponent *mRpcConfigComponent;
        std::queue<std::shared_ptr<TaskSource<bool>>> mWaitTaskQueue;
        std::unordered_map<std::string, std::shared_ptr<ProxyClient>> mServiceNodeMap;
    };
}// namespace Sentry