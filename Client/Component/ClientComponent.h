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
        explicit ClientTask(int id);
    public:
        void OnResponse(std::shared_ptr<Rpc::Packet> response) final;
        std::shared_ptr<Rpc::Packet> Await() { return this->mTask.Await(); }
    private:
        TaskSource<std::shared_ptr<Rpc::Packet>> mTask;
    };
}

namespace Client
{
    class TcpRpcClientContext;

    class ClientComponent : public RpcTaskComponent<int, Rpc::Packet>,
            public ILuaRegister,  public IRpc<Rpc::Packet>
    {
    public:
        ClientComponent();
        ~ClientComponent() final = default;
    public:
		int New(const std::string & ip, unsigned short port);
        bool Send(int id, std::shared_ptr<Rpc::Packet> request, int & rpcId);
		std::shared_ptr<Rpc::Packet> Call(int id, std::shared_ptr<Rpc::Packet> request);
        void OnMessage(std::shared_ptr<Rpc::Packet> message) final;
    protected:
        bool LateAwake() final;
        void StartClose(const std::string &address) final;
        void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
        void OnCloseSocket(const std::string &address, int code) final;
        void OnTaskComplete(int key) final { this->mNumberPool.Push(key); }
    private:
        void OnRequest(const Rpc::Packet & message);
    private:
        unsigned int mIndex;
        class ProtoComponent * mProtoComponent;
        class LuaScriptComponent * mLuaComponent;
		Util::NumberBuilder<int, 10> mNumberPool;
		std::unordered_map<long long, long long> mTimers;
		std::unordered_map<size_t, std::shared_ptr<TcpRpcClientContext>> mClients;
    };
}