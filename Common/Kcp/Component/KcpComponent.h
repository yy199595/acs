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
namespace Tendo
{
	class KcpComponent : public Component, public ISecondUpdate
	{
	public:
		KcpComponent();
		bool StartListen(const char* name);
	private:
		bool LateAwake() final;
		void OnSecondUpdate(int tick);
		static int OnServerSend(const char* buf, int len, ikcpcb* kcp, void* user);
		static int OnClientSend(const char* buf, int len, ikcpcb* kcp, void* user);
	private:
		void Main();
		void Receive();
		void Update(Asio::Context& context);
	private:
		unsigned short mPort;
		std::thread* mThread;
		ikcpcb* mKcpServer;
		Asio::Context* mContext;
		asio::ip::udp::socket* mServerSocket;
		std::array<char, 56635> mReceiveBuffer;
		asio::ip::udp::endpoint mClientEndpoint;
		std::unordered_map<std::string, IKCPCB*> mKcpClients;
	};
}


#endif //APP_KCPCOMPONENT_H
