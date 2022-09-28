//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H
#include"Client/Message.h"
#include"Tcp/TcpContext.h"
#include"Message/c2s.pb.h"
using namespace Tcp;
using namespace google::protobuf;
namespace Sentry
{
    // 网关session
	class OuterNetComponent;
 	class OuterNetClient : public Tcp::TcpContext
	{
	 public:
		OuterNetClient(std::shared_ptr<SocketProxy> socket, OuterNetComponent* component);
		~OuterNetClient() final = default;
	 public:
		void StartClose();
		void StartReceive(int second = 0);
		unsigned int GetQps() const { return this->mQps; }
		void SendData(std::shared_ptr<Rpc::Data> message);
		unsigned int GetCallCount() const { return this->mCallCount; }
	 protected:
		void OnConnect(const asio::error_code &error) {}
        void OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
        void OnTimerEnd(int timeout);
        void StartTimer(int second);
        void CloseSocket(XCode code);
	 private:
		unsigned int mQps;
        unsigned int mCallCount;
        Tcp::DecodeState mState;
        OuterNetComponent* mGateComponent;
        std::shared_ptr<Rpc::Data> mMessage;
        std::shared_ptr<asio::steady_timer> mTimer;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
