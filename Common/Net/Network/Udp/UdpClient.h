//
// Created by 64658 on 2024/10/24.
//

#ifndef APP_UDPCLIENT_H
#define APP_UDPCLIENT_H
#include "Net/Network/Tcp/Asio.h"
#include "Proto/Message/IProto.h"
using asio_udp = asio::ip::udp;
namespace udp
{
	class Client
	{
	public:
		explicit Client(asio::any_io_executor & io);
		~Client();
	public:
		bool Init(const std::string & local);
		bool Init(asio_udp::socket * sock, const std::string & remote);
	public:
		void Send(tcp::IProto * message);
	private:
		bool mDelete;
		asio_udp::socket * mSocket;
		asio::streambuf mSendBuffer;
		asio_udp::endpoint mEndPoint;
		asio::any_io_executor mExecutor;
		//char mSendBuffer[std::numeric_limits<unsigned short>::max()] = { 0 };
	};
}


#endif //APP_UDPCLIENT_H
