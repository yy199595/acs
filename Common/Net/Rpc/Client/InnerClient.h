#pragma once
#include"Core/Map/HashMap.h"
#include"Rpc/Client/Message.h"
#include"Network/Tcp/Client.h"
#include"Entity/Component/IComponent.h"
using namespace tcp;

namespace rpc
{
	struct AuthInfo
	{
	public:
		int id;
		std::string user;
		std::string passwd;
		std::string local; //本机地址
	};
}

namespace rpc
{
	class InnerClient : public tcp::Client
	{
	 public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		explicit InnerClient(int id, Component * component);
		~InnerClient() final;
	public:
		void Close();
		bool Send(rpc::Packet * message);
		void StartReceive(tcp::Socket * socket);
	 private:
		void CloseSocket(int code);
		void OnSendMessage() final;
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnConnect(bool result, int count) final;
		bool MakeMessage(const rpc::ProtoHead & header);
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code & code) final;
		void OnReceiveMessage(std::istream & is, size_t) final;
	private:
		int mSockId;
		int mDecodeStatus;
		Component * mComponent;
		rpc::ProtoHead mProtoHead;
		std::unique_ptr<rpc::Packet> mMessage;
		custom::Queue<rpc::Packet *> mSendMessages;
		custom::HashMap<int, rpc::Packet *> mWaitResMessages; //等待返回的服务器消息
	};
}// namespace Sentry