#pragma once

#include<Component/Component.h>
#include<Script/LuaTable.h>
#include<Other/MultiThreadQueue.h>
#include"Network/Rpc/ProtoRpcClient.h"
#include"Pool/AllotorPool.h"
namespace GameKeeper
{
	// 管理所有session
	class RpcClientComponent : public Component, public ISocketListen,
                               public IRpc<com::Rpc_Request, com::Rpc_Response>
	{
	public:
		RpcClientComponent() = default;
		~RpcClientComponent() override = default;
	public:
        void StartClose(long long id) final;
        void OnRequest(com::Rpc_Request * request) final;
        void OnResponse(com::Rpc_Response * response) final;
        void OnConnectAfter(long long id, XCode code) final;
        void OnCloseSocket(long long socketId, XCode code) final;

	protected:
		void OnListen(SocketProxy * socket) final;
	public:
		ProtoRpcClient * GetRpcSession(long long id);
        ProtoRpcClient * NewSession(const std::string &name);
	protected:
        bool Awake() final;
        bool LateAwake() final;
		void OnDestory() final;

	public:
		bool CloseSession(long long id);
		bool SendByAddress(long long id, com::Rpc_Request * message);
		bool SendByAddress(long long id, com::Rpc_Response * message);
	private:
		ProtoRpcClient *GetSession(long long id);
	private:
        class RpcComponent * mRpcComponent;
        AllotorPool<ProtoRpcClient> mClientPool;
        class ThreadPoolComponent * mTaskComponent;
        class RpcConfigComponent * mProtoConfigComponent;
		std::unordered_map<long long, ProtoRpcClient *> mRpcClientMap;
	};
}
