//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H
#include"Rpc/Common/Message.h"
#include"Network/Tcp/Client.h"
#include"Entity/Component/IComponent.h"

namespace rpc
{
    // 网关session
 	class OuterTcpClient final : public tcp::Client
#ifdef __SHARE_PTR_COUNTER__
			, public memory::Object<OuterTcpClient>
#endif
	{
	 public:
		typedef acs::IRpc<rpc::Message, rpc::Message> Component;
		explicit OuterTcpClient(int id, Component* component, Asio::Context & main);
		~OuterTcpClient() final;
	 public:
		void Stop();
		void Send(rpc::Message * message);
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
		void OnSendMessage(size_t size) final;
		void AddToSendQueue(rpc::Message * message);
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
		std::unique_ptr<rpc::Message> mMessage;
		std::queue<rpc::Message *> mSendMessages;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H