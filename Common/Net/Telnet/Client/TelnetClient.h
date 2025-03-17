//
// Created by 64658 on 2025/3/4.
//

#ifndef APP_TELNETCLIENT_H
#define APP_TELNETCLIENT_H
#include "Network/Tcp/Client.h"
#include "Telnet/Common/TelnetProto.h"
#include "Entity/Component/IComponent.h"

namespace telnet
{
	class Client : public tcp::Client
	{
	public:
		typedef acs::IRpc<telnet::Request, telnet::Response> Component;
		Client(int id, tcp::Socket * socket, Asio::Context & main, Component * component);
	public:
		void StartClose();
		void StartReceive();
		void Send(std::unique_ptr<telnet::Response> message);
	private:
		void OnSendMessage(size_t size) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReadError(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
	private:
		int mSocketId;
		Asio::Context & mMain;
		Component * mComponent;
		std::unique_ptr<telnet::Response> mResponse;
	};
}



#endif //APP_TELNETCLIENT_H
