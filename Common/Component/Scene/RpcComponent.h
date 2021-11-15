#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Other/MultiThreadQueue.h>
#include <Rpc/RpcConnector.h>
namespace GameKeeper
{
	// 管理所有session
	class RpcComponent : public Component, public ISocketListen
	{
	public:
		friend RpcClient;
		RpcComponent() = default;

		~RpcComponent() override = default;
	public:
		void OnCloseSession(RpcClient * socket, XCode code);
		void OnConnectRemoteAfter(RpcConnector * session, XCode code);
		void OnSendMessageAfter(RpcClient *session, std::string * message, XCode code);
    public:
		void OnRequest(RpcClient * session, com::Rpc_Request * request);
        void OnResponse(RpcClient * session, com::Rpc_Response * response);
    protected:
		void OnListen(SocketProxy * socket) final;
	public:
        RpcClient * GetRpcSession(long long id);
        RpcConnector * GetLocalSession(long long id);
        long long NewSession(const std::string &name, const std::string & ip, unsigned short port);
	protected:
		bool Awake() override;
		void OnDestory() override;

	public:
		bool CloseSession(long long id);
		bool SendByAddress(long long id, com::Rpc_Request * message);
		bool SendByAddress(long long id, com::Rpc_Response * message);
    private:
		RpcClient *GetSession(long long id);
	private:
		class TaskPoolComponent * mTaskComponent;
		class RpcProtoComponent *mProtocolComponent;
        class RpcRequestComponent * mRequestComponent;
        class RpcResponseComponent * mResponseComponent;
	private:
		std::queue<RpcClient *> mSessionQueue;
        std::unordered_map<long long, RpcClient *> mSessionAdressMap;
	};
}
