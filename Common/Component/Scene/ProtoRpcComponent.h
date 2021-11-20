#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Other/MultiThreadQueue.h>
#include"Rpc/ProtoRpcConnector.h"
namespace GameKeeper
{
	// 管理所有session
	class ProtoRpcComponent : public Component, public ISocketListen, 
		public IRpc<ProtoRpcClient, com::Rpc_Request, com::Rpc_Response>
	{
	public:
		friend RpcClient;
		ProtoRpcComponent() = default;

		~ProtoRpcComponent() override = default;
	public:
		void OnCloseSession(long long socketId, XCode code);
		void OnConnectRemoteAfter(ProtoRpcConnector * session, XCode code);
		void OnSendMessageAfter(ProtoRpcClient *session, std::string * message, XCode code);
	public:
		void OnRequest(ProtoRpcClient * session, com::Rpc_Request * request) final;
		void OnResponse(ProtoRpcClient * session, com::Rpc_Response * response) final;
	protected:
		void OnListen(SocketProxy * socket) final;
	public:
		ProtoRpcClient * GetRpcSession(long long id);
		ProtoRpcConnector * GetLocalSession(long long id);
		ProtoRpcConnector * NewSession(const std::string &name);
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
		class TaskPoolComponent * mTaskComponent;
		class RpcConfigComponent *mProtocolComponent;
		class RpcRequestComponent * mRequestComponent;
		class RpcResponseComponent * mResponseComponent;
	private:
		std::queue<RpcClient *> mSessionQueue;
		std::unordered_map<long long, ProtoRpcClient *> mSessionAdressMap;
	};
}
