#pragma once

#include <istream>
#include <Network/SessionBase.h>
namespace Sentry
{
	class TelnetClientSession : public SessionBase
	{
	public:
		TelnetClientSession(ISocketHandler * hanlder);
		~TelnetClientSession();
	protected:
		void OnStartReceive() override;
	private:
		void ReadHandler(const asio::error_code & err, const size_t size);
	private:
        const std::string mDelim;
		asio::streambuf mReceiveBuffer;
	};
}