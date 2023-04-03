#pragma once
#include<queue>
#include"Rpc/Client/Message.h"
#include"Util/Json/JsonWriter.h"
#include"Rpc/Client/InnerNetClient.h"
#include"Server/Component/TcpListenerComponent.h"

namespace Sentry
{
	// 管理内网rpc的session
	class InnerNetComponent : public TcpListenerComponent,
							  public IRpc<Rpc::Packet>, public IServerRecord, public IFrameUpdate
	{
	 public:
		InnerNetComponent();
		~InnerNetComponent() override = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnMessage(std::shared_ptr<Rpc::Packet> message) final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnCloseSocket(const std::string & address, int code) final;
		void OnSendFailure(const std::string& address, std::shared_ptr<Rpc::Packet> message) final;
	 protected:
        bool LateAwake() final;
		void OnDestroy() final;
		void OnFrameUpdate(float t) final;
        void OnRecord(Json::Writer & document) final;
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
	 public:
        InnerNetClient * GetSession(const std::string& address);
        InnerNetClient * GetOrCreateSession(const std::string& address);
		const NodeInfo * GetNodeInfo(const std::string & address) const;
	public:
		bool Send(const std::shared_ptr<Rpc::Packet>& message); //发送到本地
		bool Send(const std::string & address, const std::shared_ptr<Rpc::Packet>& message);
	public:
		size_t GetConnectClients(std::vector<std::string> & list) const; //获取所有连接进来的客户端
		size_t Broadcast(const std::shared_ptr<Rpc::Packet>& message) const; //广播给所有链接进来的客户端
	private:
        bool IsAuth(const std::string & address);          
        bool OnAuth(const std::shared_ptr<Rpc::Packet>& message);
	 private:
		int mMaxHandlerCount;
		std::string mLocation;
		unsigned int mSumCount;
        class ThreadComponent * mNetComponent;
        class InnerNetMessageComponent* mMessageComponent;
        std::unordered_map<std::string, std::string> mUserMaps;
		std::queue<std::shared_ptr<Rpc::Packet>> mWaitMessages;
		std::unordered_map<std::string, NodeInfo> mLocationMaps;
		std::unordered_map<std::string, std::shared_ptr<InnerNetClient>> mRpcClientMap;
    };
}
