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
	class LuaScriptComponent;
	class NetThreadComponent;
}
namespace Client
{
    class ClientRpcTask;
    class TcpRpcClientContext;

    class ClientComponent : public Component,
							public IRpc<c2s::Rpc::Call, c2s::Rpc::Response>, public IComplete, public ILuaRegister
    {
    public:
        ClientComponent();

        ~ClientComponent() final = default;

    public:
        void StartClose(const std::string & address) final {};

        void OnCloseSocket(const std::string & address, XCode code) final {};

        void OnRequest(std::shared_ptr<c2s::Rpc::Call> t1) final;

        void OnResponse(std::shared_ptr<c2s::Rpc_Response> t2) final;

    public:
		bool StartConnect(const std::string & ip, unsigned short port);
		std::shared_ptr<c2s::Rpc::Response> Call(std::shared_ptr<c2s::Rpc::Request> request);
    public:
        unsigned int AddRpcTask(std::shared_ptr<ClientRpcTask> task, int ms);

    protected:

        bool LateAwake() final;

		void OnAllServiceStart() override;

		void OnTimeout(long long rpcId);

		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
    private:
        unsigned short mPort;
        TimerComponent *mTimerComponent;
		LuaScriptComponent * mLuaComponent;
        std::shared_ptr<TcpRpcClientContext> mTcpClient;
        std::unordered_map<long long, TaskSourceShared<c2s::Rpc::Response>> mRpcTasks;
    };
}