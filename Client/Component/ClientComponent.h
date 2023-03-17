#pragma once
#include<memory>
#include"Client/Message.h"
#include"Message/c2s.pb.h"
#include"Lua/LocalTable.h"
#include"Source/TaskSource.h"
#include"Component/RpcTaskComponent.h"
using namespace Sentry;
using namespace google::protobuf;

namespace Sentry
{
    class TimerComponent;
    class HttpComponent;
	class LuaScriptComponent;
	class ThreadComponent;
}

namespace Client
{
    class ClientTask : public Sentry::IRpcTask<Rpc::Packet>
    {
    public:
        ClientTask(int id);
    public:
        void OnResponse(std::shared_ptr<Rpc::Packet> response);
        std::shared_ptr<Rpc::Packet> Await() { return this->mTask.Await(); }
    private:
        int mTaskId;
        TaskSource<std::shared_ptr<Rpc::Packet>> mTask;
    };
}

namespace Client
{
    class TcpRpcClientContext;

    class ClientComponent : public RpcTaskComponent<long long, Rpc::Packet>,
            public ILuaRegister,  public IRpc<Rpc::Packet>
    {
    public:
        ClientComponent();

        ~ClientComponent() final = default;
    public:
        void OnRequest(std::shared_ptr<c2s::rpc::call> t1);
		int New(const std::string & ip, unsigned short port);
		std::shared_ptr<Rpc::Packet> Call(int id, std::shared_ptr<Rpc::Packet> request);
        void OnMessage(std::shared_ptr<Rpc::Packet> message) final;
    protected:
        bool LateAwake() final;
        void StartClose(const std::string &address) final;
        void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
        void OnCloseSocket(const std::string &address, int code) final;
    private:
        unsigned int mIndex;
        TimerComponent *mTimerComponent;
		Util::NumberBuilder<int, 10> mNumberPool;
		std::unordered_map<long long, long long> mTimers;
		std::unordered_map<size_t, std::shared_ptr<TcpRpcClientContext>> mClients;
    };
}