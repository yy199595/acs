#pragma once

#include"Lua/Table.h"
#include"Client/Message.h"
#include"Client/InnerNetClient.h"
#include"Listener/TcpListenerComponent.h"

namespace Sentry
{
    class InnerServerNode
    {
    public:
        std::string UserName;
        std::string PassWord;
        std::string Location;
    };
}

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
        const std::string & GetUser() const { return this->mUserName; };
        const std::string & GetPassword() const { return this->mPassword; }
		std::shared_ptr<InnerNetClient> GetSession(const std::string& address);
		std::shared_ptr<InnerNetClient> GetOrCreateSession(const std::string& address);
	 public:
		bool Send(const std::string & address, std::shared_ptr<Rpc::Data> message);
	private:
        bool IsAuth(const std::string & address);
        bool OnForward(std::shared_ptr<Rpc::Data> message);
        bool OnBroadcast(std::shared_ptr<Rpc::Data> message);
        bool OnAuth(const std::string & address, std::shared_ptr<Rpc::Data> message);
        bool OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message);
		bool OnResponse(const std::string& address, std::shared_ptr<Rpc::Data> message);
	 private:
        std::string mUserName;
        std::string mPassword;
        class NetThreadComponent * mNetComponent;
        class OuterNetComponent * mOuterComponent;
        class InnerNetMessageComponent* mMessageComponent;
        std::unordered_map<std::string, std::string> mUserMaps;
        std::unordered_map<std::string, InnerServerNode> mLocationMaps;
        std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mRpcClientMap;
	};
}
