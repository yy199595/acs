#pragma once
#include<Component/Component.h>
#include"Async/TaskSource.h"
#include"Protocol/c2s.pb.h"
using namespace Sentry;
using namespace google::protobuf;

namespace Sentry
{
    class TimerComponent;
    class HttpClientComponent;
    class ThreadPoolComponent;
}
namespace Client
{
    class ClientRpcTask;
    class TcpRpcClient;

    class ClientComponent : public Component,
                            public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>, public IComplete
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
        XCode Call(const std::string & name, std::shared_ptr<Message> response);
        XCode Call(const std::string & name, const Message & message, std::shared_ptr<Message> response);
    public:
        unsigned int AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms);

    protected:
        bool Awake() final;

        bool LateAwake() final;

		void OnComplete() override;

		void OnTimeout(long long rpcId);

    private:
        std::string mIp;
        unsigned short mPort;
        TaskComponent * mTaskComponent;
        TimerComponent *mTimerComponent;
        HttpClientComponent * mHttpComponent;
        std::shared_ptr<TcpRpcClient> mTcpClient;
        //std::unordered_map<long long, std::shared_ptr<TaskSource<std::shared_ptr<c2s::Rpc_Response>>>> mRpcTasks;
    };
}