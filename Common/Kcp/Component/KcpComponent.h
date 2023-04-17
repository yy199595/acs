//
// Created by MyPC on 2023/4/7.
//

#ifndef APP_KCPCOMPONENT_H
#define APP_KCPCOMPONENT_H
#include<thread>
#include<unordered_map>
#include"Kcp/Common/ikcp.h"
#include"Network/Tcp/Asio.h"
#include"Entity/Component/Component.h"
#include "Rpc/Client/Message.h"

using AsioTimer = asio::steady_timer;
using AsioUdpSocket = asio::ip::udp::socket;
using AsioUdpEndpoint = asio::ip::udp::endpoint;

struct KcpClient
{
	ikcpcb * kcp;
	AsioUdpEndpoint endpoint;
};
namespace Tendo
{
	class KcpComponent : public Component
	{
	public:
		KcpComponent();
		bool StartListen(const char* name);
	private:
		bool LateAwake() final;
		static int OnServerSend(const char* buf, int len, ikcpcb* kcp, void* user);
		static int OnClientSend(const char* buf, int len, ikcpcb* kcp, void* user);
	private:
		void Start();
		void Receive();
		void Update(const asio::error_code & code);
	private:
		void OnMessage(std::shared_ptr<Msg::Packet> message);
	private:
		unsigned short mPort;
		ikcpcb* mKcpServer;
		Asio::Context* mContext;
		AsioUdpEndpoint mEndpoint;
		asio::streambuf mRecvBuffer;
		std::unique_ptr<AsioTimer> mTimer;
		std::array<char, 65535> mReceiveBuffer;
		std::unique_ptr<AsioUdpSocket> mUdpSocket;
		std::unordered_map<unsigned int, KcpClient *> mKcpClients;
	};
}


#endif //APP_KCPCOMPONENT_H
