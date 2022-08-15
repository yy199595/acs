//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H
#include"Network/TcpContext.h"
#include"Message/c2s.pb.h"
using namespace Tcp;
using namespace google::protobuf;
namespace Sentry
{
	class RpcGateComponent;
 	class GateMessageClient : public Tcp::TcpContext
	{
	 public:
		GateMessageClient(std::shared_ptr<SocketProxy> socket, RpcGateComponent* component);
		~GateMessageClient() final = default;
	 public:
		void StartClose();
		void StartReceive();
		unsigned int GetQps() const { return this->mQps; }
		void SendToClient(std::shared_ptr<c2s::rpc::call> message);
		void SendToClient(std::shared_ptr<c2s::rpc::response> message);
		unsigned int GetCallCount() const { return this->mCallCount; }
	 protected:
		void OnConnect(const asio::error_code &error) {}
        void OnReceiveLength(const asio::error_code &code, int length) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & readStream, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
		void CloseSocket(XCode code);
	 private:
		unsigned int mQps;
        unsigned int mCallCount;
		RpcGateComponent* mGateComponent;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
