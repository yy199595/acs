#pragma once

#include <istream>
#include <Network/SessionBase.h>
namespace GameKeeper
{
	class TelnetClientSession : public SessionBase
	{
	public:
		TelnetClientSession(ISocketHandler * handler);
		~TelnetClientSession();

    public:
        SocketType GetSocketType() override { return SocketType::RemoteSocket;}
	protected:
		void OnSessionEnable() override;
	private:
		void ReadHandler(const asio::error_code & err, const size_t size);
	private:
        const std::string mDelim;
		asio::streambuf mReceiveBuffer;
	};
}