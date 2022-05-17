#pragma once
#include<Component/Component.h>
#include"Async/TaskSource.h"
#include"Protocol/c2s.pb.h"
using namespace Sentry;
using namespace google::protobuf;

namespace Sentry
{
    class TimerComponent;
    class HttpComponent;
    class NetThreadComponent;
}
namespace Client
{
    class ClientRpcTask;
    class TcpRpcClient;

    class ClientComponent : public Component,
 		public IRpc<c2s::Rpc::Request, c2s::Rpc::Response>, public IComplete
    {
    public:
        ClientComponent();

        ~ClientComponent() final = default;

    public:
        void StartClose(const std::string & address) final {};

        void OnCloseSocket(const std::string & address, XCode code) final {};

        void OnConnectAfter(const std::string & address, XCode code) final {}

        void OnRequest(std::shared_ptr<c2s::Rpc_Request> t1) final;

        void OnResponse(std::shared_ptr<c2s::Rpc_Response> t2) final;

	 private:
		void StartClient(const std::string & account);
		std::shared_ptr<TcpRpcClient> GetCurrentRpcClient();
		bool GetClient(const std::string & account, const std::string & passwd);
    public:
		XCode Call(const std::string & name);
		XCode Call(const std::string & name, const Message & request);
		XCode Call(const std::string & name, std::shared_ptr<Message> response);
        XCode Call(const std::string & name, const Message & message, std::shared_ptr<Message> response);
    public:
        unsigned int AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms);

    protected:

        bool LateAwake() final;

		void OnAllServiceStart() override;

		void OnTimeout(long long rpcId);

    private:
        std::string mIp;
        unsigned short mPort;
        TaskComponent * mTaskComponent;
        TimerComponent *mTimerComponent;
        HttpComponent * mHttpComponent;
        //std::shared_ptr<TcpRpcClient> mTcpClient;
		std::unordered_map<unsigned int, std::shared_ptr<TcpRpcClient>> mClients;
        std::unordered_map<long long, TaskSourceShared<c2s::Rpc::Response>> mRpcTasks;
    };
}