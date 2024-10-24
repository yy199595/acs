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
	{
	 public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		explicit OuterClient(int id, Component* component);
		~OuterClient() final = default;
	 public:
		void Stop(int code);
		bool Send(rpc::Packet * message);
		inline int GetSockId() const { return this->mSockId; }
		void StartReceive(tcp::Socket * socket, int second = 0);
	protected:
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnReadError(const Asio::Code &code) final;
        void OnReceiveMessage(std::istream & readStream, size_t) final;
	private:
        void CloseSocket(int code);
		void OnSendMessage() final;
		void OnSendMessage(const asio::error_code &code) final;
	private:
		int mSockId;
		int mDecodeState;
		unsigned int mMaxQps;
		long long mLastRecvTime;
		Component * mComponent;
		rpc::Packet * mMessage;
		rpc::ProtoHead mProtoHead;
		custom::Queue<rpc::Packet *> mSendMessages;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
