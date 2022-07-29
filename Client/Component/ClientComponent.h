#pragma once
#include"Message/c2s.pb.h"
#include"Async/TaskSource.h"
#include"Component/Rpc/RpcTaskComponent.h"
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
    class ClientTask : public Sentry::IRpcTask<c2s::Rpc::Response>
    {
    public:
        ClientTask(int ms);
    public:
        void OnTimeout() final;
        long long GetRpcId() { return this->mTaskId; }
        void OnResponse(std::shared_ptr<c2s::Rpc::Response> response);
        std::shared_ptr<c2s::Rpc::Response> Await() { return this->mTask.Await(); }
    private:
        long long mTaskId;
        TaskSource<std::shared_ptr<c2s::Rpc::Response>> mTask;
    };
}

namespace Client
{
    class TcpRpcClientContext;

    class ClientComponent : public RpcTaskComponent<c2s::Rpc::Response>, public IComplete, public ILuaRegister
    {
    public:
        ClientComponent();

        ~ClientComponent() final = default;

    public:
        void OnRequest(std::shared_ptr<c2s::Rpc::Call> t1);
		bool StartConnect(const std::string & ip, unsigned short port);
		std::shared_ptr<c2s::Rpc::Response> Call(std::shared_ptr<c2s::Rpc::Request> request);
    protected:

        bool LateAwake() final;
		void OnAllServiceStart() final;
		void OnTimeout(long long rpcId);
        void OnAddTask(RpcTask task) final;
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
    private:
        unsigned short mPort;
        TimerComponent *mTimerComponent;
		LuaScriptComponent * mLuaComponent;
        std::shared_ptr<TcpRpcClientContext> mTcpClient;
    };
}