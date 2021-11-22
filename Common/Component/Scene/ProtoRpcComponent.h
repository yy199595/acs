#pragma once

#include<Component/Component.h>
#include<Script/LuaTable.h>
#include<Other/MultiThreadQueue.h>
#include"Network/Rpc/ProtoRpcClient.h"
#include"Pool/AllotorPool.h"
namespace GameKeeper
{
	// 管理所有session
	class ProtoRpcComponent : public Component, public ISocketListen, 
		public IRpc<com::Rpc_Request, com::Rpc_Response>
	{
	public:
		ProtoRpcComponent() = default;
		~ProtoRpcComponent() override = default;
	public:
        void StartClose(long long id) final;
        void OnConnectAfter(long long id, XCode code) final;
        void OnCloseSocket(long long socketId, XCode code) final;
		void OnRequest(long long id, com::Rpc_Request * request) final;
		void OnResponse(long long id, com::Rpc_Response * response) final;
	protected:
		void OnListen(SocketProxy * socket) final;
	public:
		ProtoRpcClient * GetRpcSession(long long id);
        ProtoRpcClient * NewSession(const std::string &name);
	protected:
		bool Awake() override;
		void OnDestory() override;

	public:
		bool CloseSession(long long id);
		bool SendByAddress(long long id, com::Rpc_Request * message);
		bool SendByAddress(long long id, com::Rpc_Response * message);
	private:
		ProtoRpcClient *GetSession(long long id);
	private:
        AllotorPool<ProtoRpcClient> mClientPool;
        class TaskPoolComponent * mTaskComponent;
        class RpcConfigComponent *mProtocolComponent;
		class RpcRequestComponent * mRequestComponent;
		class RpcResponseComponent * mResponseComponent;
		std::unordered_map<long long, ProtoRpcClient *> mSessionAdressMap;
	};
}
