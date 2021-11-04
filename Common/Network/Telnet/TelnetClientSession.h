#pragma once

#include <istream>
#include <Network/SocketProxy.h>
namespace GameKeeper
{
	class TelnetClientComponent;
	class TelnetClientSession
	{
	public:
		 explicit TelnetClientSession(TelnetClientComponent * handler);
		~TelnetClientSession() = default;

    public:
        SocketType GetSocketType() { return SocketType::RemoteSocket;}
	private:
		void ReadHandler(const asio::error_code & err, const size_t size);
	private:
        const std::string mDelim;
		asio::streambuf mReceiveBuffer;
	};
}