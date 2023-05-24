#pragma once
#include<memory>
#include"Rpc/Client/Message.h"
#include"Async/Source/TaskSource.h"
#include"Rpc/Component/RpcTaskComponent.h"
using namespace Tendo;

namespace Tendo
{
    class TimerComponent;
    class HttpComponent;
	class LuaScriptComponent;
	class ThreadComponent;
}

namespace Client
{
    class ClientTask : public Tendo::IRpcTask<Msg::Packet>
    {
    public:
        explicit ClientTask(int id);
    public:
        void OnResponse(std::shared_ptr<Msg::Packet> response) final;
        std::shared_ptr<Msg::Packet> Await() { return this->mTask.Await(); }
    private:
        TaskSource<std::shared_ptr<Msg::Packet>> mTask;
    };
	//typedef TaskSource<std::shared_ptr<Rpc::Packet>> ClientTask;
}

namespace Tendo
{
	class ProtoComponent;
	class LuaScriptComponent;
}

namespace Client
{
    class TcpRpcClientContext;

    class ClientComponent final : public RpcTaskComponent<int, Msg::Packet>,
            public ILuaRegister, public IDestroy, public IRpc<Msg::Packet>
    {
    public:
        ClientComponent();
        ~ClientComponent() final = default;
        ClientComponent(const ClientComponent &) = delete;
        ClientComponent(const ClientComponent &&) = delete;
    public:
		bool Close(int id);
		int New(const std::string & ip, unsigned short port);
        bool Send(int id, const std::shared_ptr<Msg::Packet>& request, int & rpcId);
		std::shared_ptr<Msg::Packet> Call(int id, const std::shared_ptr<Msg::Packet> & request, bool async = true);
        void OnMessage(std::shared_ptr<Msg::Packet> message) final;
    protected:
        bool LateAwake() final;
        void StartClose(const std::string &address) final;
        void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
        void OnCloseSocket(const std::string &address, int code) final;
        void OnTaskComplete(int key) final { this->mNumberPool.Push(key); }
    private:
        void OnRequest(const Msg::Packet & message) const;
		void OnDestroy() final;
    private:
        unsigned int mIndex;
		ProtoComponent * mProtoComponent;
		LuaScriptComponent * mLuaComponent;
		Util::NumberBuilder<int, 10> mNumberPool;
		std::unordered_map<long long, long long> mTimers;
		std::unordered_map<size_t, std::shared_ptr<TcpRpcClientContext>> mClients;
    };
}