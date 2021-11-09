#pragma once

#include <thread>
#include <Component/Component.h>
#include <Script/LuaTable.h>
#include <Other/MultiThreadQueue.h>
#include <Network/Rpc/RpcLocalSession.h>
namespace GameKeeper
{
	// 管理所有session
	class RpcComponent : public Component, public ISocketListen
	{
	public:
		friend RpcClientSession;
		RpcComponent() = default;

		~RpcComponent() override = default;
	public:
		void OnCloseSession(RpcClientSession * socket, XCode code);
		void OnConnectRemoteAfter(RpcLocalSession * session, XCode code);
		void OnReceiveMessage(RpcClientSession * session, string * message);
		void OnSendMessageAfter(RpcClientSession *session, std::string * message, XCode code);
		
    protected:
		void OnListen(SocketProxy * socket) final;
	public:
        RpcLocalSession * GetLocalSession(long long id);
        RpcClientSession * GetRemoteSession(long long id);
        long long NewSession(const std::string &name, const std::string & ip, unsigned short port);
	protected:
		bool Awake() override;
		void OnDestory() override;

	public:
		bool CloseSession(long long id);
		bool SendByAddress(long long id, std::string * message);
		bool SendByAddress(long long id, com::DataPacket_Request & message);
		bool SendByAddress(long long id, com::DataPacket_Response & message);

    public:
        std::string * Serialize(const com::DataPacket_Request & message);
        std::string * Serialize(const com::DataPacket_Response & message);
    private:
		
		RpcClientSession *GetSession(long long id);
		bool OnReceive(RpcClientSession *, const std::string & message);

	private:
		class TaskPoolComponent * mTaskComponent;
		class RpcComponent *mNetWorkManager;
		class ProtocolComponent *mProtocolComponent;
        class RpcRequestComponent * mRequestComponent;
        class RpcResponseComponent * mResponseComponent;
	private:		
        char mMessageBuffer[1024 * 1024];
        com::DataPacket_Request mRequestData;
        com::DataPacket_Response mResponseData;
		std::queue<std::string> mRecvSessionQueue;
		std::queue<RpcClientSession *> mSessionQueue;
        std::unordered_map<long long, RpcClientSession *> mSessionAdressMap;
	};
}
