﻿#pragma once

#include"Lua/Table.h"
#include"Component/Component.h"
#include"Client/InnerNetClient.h"
#include"Listener/TcpServerListener.h"
namespace Sentry
{
	// 管理内网rpc的session
	class InnerNetComponent : public Component, public TcpServerListener,
                              public IRpc<Tcp::BinMessage>
	{
	 public:
		InnerNetComponent() = default;
		~InnerNetComponent() override = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
        void OnMessage(const std::string & address, std::shared_ptr<Tcp::BinMessage> message) final;
	 protected:
        void Awake() final;
        bool LateAwake() final;
		bool OnListen(std::shared_ptr<SocketProxy> socket) final;
	 public:
		std::shared_ptr<InnerNetClient> GetSession(const std::string& address);
		std::shared_ptr<InnerNetClient> GetOrCreateSession(const std::string& address);
	 public:
		bool Send(const std::string & address, std::shared_ptr<com::rpc::request> message);
		bool Send(const std::string & address, std::shared_ptr<com::rpc::response> message);
	private:
		bool OnRequest(const std::string & address, const Tcp::BinMessage& message);
		bool OnResponse(const std::string& address, const Tcp::BinMessage& message);
	 private:
        class NetThreadComponent * mNetComponent;
        class InnerNetMessageComponent* mMessageComponent;
        std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mRpcClientMap;
	};
}