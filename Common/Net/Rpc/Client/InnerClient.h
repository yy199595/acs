#pragma once
#include"Core/Map/HashMap.h"
#include"Rpc/Client/Message.h"
#include"Network/Tcp/Client.h"
#include"Entity/Component/IComponent.h"
using namespace tcp;

namespace rpc
{
	class InnerClient : public tcp::Client
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<InnerClient>
#endif
	{
	 public:
		typedef acs::IRpc<rpc::Message, rpc::Message> Component;
		explicit InnerClient(int id, Component * component, bool client, Asio::Context &);
		~InnerClient() final;
	public:
		void Close();
		bool Send(rpc::Message * message);
		void StartReceive(tcp::Socket * socket);
	 private:
		void CloseSocket();
		void CloseSocket(int code);
		void OnSendMessage() final;
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnConnect(bool result, int count) final;
		bool MakeMessage(const rpc::ProtoHead & header);
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code & code) final;
		void OnReceiveMessage(std::istream & is, size_t, const Asio::Code & code) final;
	private:
		bool mClose;
		const int mSockId;
		int mDecodeStatus;
		const bool mIsClient;
		Component * mComponent;
		rpc::ProtoHead mProtoHead;
		Asio::Context & mMainContext;
		std::unique_ptr<rpc::Message> mMessage;
		std::queue<rpc::Message *> mSendMessages;
		std::unordered_map<int, rpc::Message *> mWaitResMessages; //等待返回的服务器消息
	};
}// namespace Sentry