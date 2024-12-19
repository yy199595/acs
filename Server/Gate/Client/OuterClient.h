//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H
#include"Rpc/Client/Message.h"
#include"Network/Tcp/Client.h"
#include"Entity/Component/IComponent.h"

namespace rpc
{
    // 网关session
 	class OuterClient final : public tcp::Client
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<OuterClient>
#endif
	{
	 public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		explicit OuterClient(int id, Component* component, Asio::Context & main);
		~OuterClient() final;
	 public:
		void Stop();
		bool Send(rpc::Packet * message);
		void StartReceive(tcp::Socket * socket, int second = 0);
		inline void BindPlayer(long long id) { this->mPlayerId = id; }
		inline long long GetPlayerId() const { return this->mPlayerId; }
	protected:
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnReadError(const Asio::Code &code) final;
        void OnReceiveMessage(std::istream & readStream, size_t, const Asio::Code & code) final;
	private:
        void CloseSocket();
		void SendFirstMessage();
		void CloseSocket(int code);
		void OnSendMessage() final;
		void OnSendMessage(const asio::error_code &code) final;
	private:
		bool mClose;
		int mDecodeState;
		const int mSockId;
		long long mPlayerId;
		long long mLastRecvTime;
		Component * mComponent;
		rpc::ProtoHead mProtoHead;
		Asio::Context & mMainContext;
		std::unique_ptr<rpc::Packet> mMessage;
		std::queue<rpc::Packet *> mSendMessages;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
