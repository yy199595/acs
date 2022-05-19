//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_RPCPROXYCLIENT_H
#define GAMEKEEPER_RPCPROXYCLIENT_H
#include"Network/TcpContext.h"
#include"Protocol/c2s.pb.h"
using namespace Tcp;
using namespace google::protobuf;
namespace Sentry
{
	class GateClientComponent;
	class GateRpcClientContext : public TcpContext
	{
	 public:
		GateRpcClientContext(std::shared_ptr<SocketProxy> socket, GateClientComponent* component);
		~GateRpcClientContext() final = default;
	 public:
		void StartClose();
		void StartReceive();
		unsigned int GetQps() const { return this->mQps; }
		bool SendToClient(std::shared_ptr<c2s::Rpc::Call> message);
		bool SendToClient(std::shared_ptr<c2s::Rpc::Response> message);
		unsigned int GetCallCount() const
		{
			return this->mCallCount;
		}
	 protected:
		void OnConnect(const asio::error_code &error) {}
		bool OnRecvMessage(const asio::error_code &code, const char *message, size_t size) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
		void CloseSocket(XCode code);
	 private:
		unsigned int mQps;
		unsigned int mCallCount;
		GateClientComponent* mGateComponent;
	};
}


#endif //GAMEKEEPER_RPCPROXYCLIENT_H
