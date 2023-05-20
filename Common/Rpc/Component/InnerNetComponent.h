#pragma once
#include<queue>
#include"Rpc/Client/Message.h"
#include"Util/Json/JsonWriter.h"
#include"Rpc/Client/InnerNetTcpClient.h"
#include"Server/Component/TcpListenerComponent.h"
struct lua_State;
namespace Tendo
{
	// 管理内网rpc的session
	class InnerNetComponent : public TcpListenerComponent, public IRpc<Msg::Packet>,
			public IServerRecord, public IFrameUpdate, public IDestroy
	{
	 public:
		InnerNetComponent();
		~InnerNetComponent() override = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnMessage(std::shared_ptr<Msg::Packet> message) final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnCloseSocket(const std::string & address, int code) final;
		void OnSendFailure(const std::string& address, std::shared_ptr<Msg::Packet> message) final;
	 protected:
        bool LateAwake() final;
		void OnDestroy() final;
		void OnFrameUpdate(float t) final;
        void OnRecord(Json::Writer & document) final;
		void OnListen(std::shared_ptr<Tcp::SocketProxy> socket) final;
	 private:
		InnerNetTcpClient * GetLocalClient(const std::string& address); //本地tcp客户端
		InnerNetTcpClient * GetRemoteClient(const std::string& address); //远程tcp客户端
	public:
		bool Send(const std::shared_ptr<Msg::Packet>& message); //发送到本地
		bool Send(const std::string & address, const std::shared_ptr<Msg::Packet>& message);
        bool Send(const std::string & address, int code, const std::shared_ptr<Msg::Packet>& pack);
        bool Send(const std::string & address, const std::shared_ptr<Msg::Packet>& message, int & id);
        std::shared_ptr<Msg::Packet> Call(const std::string & address, const std::shared_ptr<Msg::Packet> & message);
	public:
		int LuaCall(lua_State * lua, const std::string & address, const std::shared_ptr<Msg::Packet> & message);
	public:
		size_t GetConnectClients(std::vector<std::string> & list) const; //获取所有连接进来的客户端
		size_t Broadcast(const std::shared_ptr<Msg::Packet>& message) const; //广播给所有链接进来的客户端
	private:
        bool IsAuth(const std::string & address);
        bool OnAuth(const std::shared_ptr<Msg::Packet>& message);
	 private:
		int mMaxHandlerCount;
		std::string mLocation;
		unsigned int mSumCount;
        class ThreadComponent * mNetComponent;
        class DispatchComponent* mMessageComponent;
        std::unordered_map<std::string, std::string> mUserMaps;
		std::queue<std::shared_ptr<Msg::Packet>> mWaitMessages;
		std::unordered_map<std::string, NodeInfo> mLocationMaps;
		std::unordered_map<std::string, std::shared_ptr<InnerNetTcpClient>> mLocalClients; //本地客户端(连接别的)
		std::unordered_map<std::string, std::shared_ptr<InnerNetTcpClient>> mRemoteClients; //远程客户端(连接过来的)
		//std::unordered_map<std::string, std::shared_ptr<InnerNetTcpClient>> mWaitAuthClients; //等待验证客户端
	};
}
