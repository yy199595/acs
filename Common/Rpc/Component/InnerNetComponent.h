#pragma once

#include"Lua/Table.h"
#include"Client/Message.h"
#include"Client/InnerNetClient.h"
#include"Listener/TcpListenerComponent.h"
namespace Sentry
{
	// 管理内网rpc的session
	class InnerNetComponent : public TcpListenerComponent, public IRpc<Rpc::Data>
	{
	 public:
		InnerNetComponent() = default;
		~InnerNetComponent() override = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
        void OnMessage(const std::string & address, std::shared_ptr<Rpc::Data> message) final;
	 protected:
        void Awake() final;
        bool LateAwake() final;
		bool OnListen(std::shared_ptr<SocketProxy> socket) final;
	 public:
		std::shared_ptr<InnerNetClient> GetSession(const std::string& address);
		std::shared_ptr<InnerNetClient> GetOrCreateSession(const std::string& address);
	 public:
		bool Send(const std::string & address, std::shared_ptr<Rpc::Data> message);
	private:
		bool OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message);
		bool OnResponse(const std::string& address, std::shared_ptr<Rpc::Data> message);
	 private:
        class NetThreadComponent * mNetComponent;
        class InnerNetMessageComponent* mMessageComponent;
        std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mRpcClientMap;
	};
}
