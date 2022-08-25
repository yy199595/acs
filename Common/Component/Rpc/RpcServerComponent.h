#pragma once

#include"Component/Component.h"
#include"Script/Table.h"
#include"Network/Rpc/MessageRpcClient.h"
#include"Network/Listener/TcpServerListener.h"
namespace Sentry
{
	// 管理所有session
	class RpcServerComponent : public Component, public TcpServerListener,
                               public IRpc<com::rpc::request, com::rpc::response>
	{
	 public:
		RpcServerComponent() = default;
		~RpcServerComponent() override = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
		void OnRequest(std::shared_ptr<com::rpc::request> request) final;
		void OnResponse(std::shared_ptr<com::rpc::response> response) final;

	 protected:
        void Awake() final;
        bool LateAwake() final;
		bool OnListen(std::shared_ptr<SocketProxy> socket) final;
	 public:
		std::shared_ptr<MessageRpcClient> GetSession(const std::string& address);
		std::shared_ptr<MessageRpcClient> GetOrCreateSession(const std::string& address);
	 public:
		bool Send(const std::string & address, std::shared_ptr<com::rpc::request> message);
		bool Send(const std::string & address, std::shared_ptr<com::rpc::response> message);
	 private:
		class TcpRpcComponent* mRpcComponent;
        class NetThreadComponent * mNetComponent;
        std::unordered_map<std::string, std::shared_ptr<MessageRpcClient>> mRpcClientMap;
	};
}
