#pragma once
#include<Component/Component.h>

using namespace Sentry;
using namespace google::protobuf;

namespace Sentry
{
    class TimerComponent;
    class ThreadPoolComponent;
}
namespace Client
{
    class ClientRpcTask;
    class TcpRpcClient;

    class ClientComponent : public Component,
                            public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>, public IStart
    {
    public:
        ClientComponent();

        ~ClientComponent() final = default;

    public:
        void StartClose(long long id) final {};

        void OnCloseSocket(long long id, XCode code) final {};

        void OnConnectAfter(long long id, XCode code) final {}

        void OnRequest(std::shared_ptr<c2s::Rpc_Request> t1) final;

        void OnResponse(std::shared_ptr<c2s::Rpc_Response> t2) final;

    public:
        unsigned int AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms);

    protected:
        bool Awake() final;

        void OnStart() final;

        bool LateAwake() final;

        void OnTimeout(long long rpcId);

    private:
        TimerComponent *mTimerComponent;
        std::shared_ptr<TcpRpcClient> mTcpClient;
        std::unordered_map<long long, std::shared_ptr<ClientRpcTask>> mRpcTasks;
    };
}