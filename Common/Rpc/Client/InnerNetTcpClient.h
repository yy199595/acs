#pragma once
#include"Rpc/Client/Message.h"
#include"Network/Tcp/TcpContext.h"
#include"Entity/Component/IComponent.h"
using namespace Tcp;
namespace Tendo
{
	struct AuthInfo
	{
	public:
		std::string UserName;
		std::string PassWord;
		std::string ServerName;
		std::string RpcAddress;
	};
}
namespace Tendo
{
	class InnerNetTcpClient : public Tcp::TcpContext
	{
	 public:
		explicit InnerNetTcpClient(IRpc<Msg::Packet> * component,
			std::shared_ptr<Tcp::SocketProxy> socket);
		explicit InnerNetTcpClient(IRpc<Msg::Packet> * component,
			std::shared_ptr<Tcp::SocketProxy> socket, AuthInfo info);
		~InnerNetTcpClient() override = default;
	 public:
		void StartClose();
		void StartReceive();
		bool IsClient() const { return this->mIsClient; }
		bool Send(const std::shared_ptr<Msg::Packet>& message, bool async = true);
		std::shared_ptr<Msg::Packet> Call(const std::shared_ptr<Msg::Packet>& message); //同步读取
	 private:
        void Update();
        void CloseSocket(int code);
        void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
		bool mIsClient;
		AuthInfo mAuthInfo;
        Tcp::DecodeState mState;
        IRpc<Msg::Packet> * mComponent;
        std::shared_ptr<Msg::Packet> mMessage;
        std::unique_ptr<asio::steady_timer> mTimer;
		//std::unordered_map<int, std::shared_ptr<Msg::Packet>> mWaitResMessages; //等待返回的消息
	};
}// namespace Sentry