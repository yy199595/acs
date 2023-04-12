
#include"TelnetClientContext.h"
#include"Util/String/StringHelper.h"
#include"Entity/Unit/App.h"
#include"Telnet/Component/ConsoleComponent.h"
namespace Tcp
{
	TelnetClientContext::TelnetClientContext(const std::shared_ptr<SocketProxy>& socketProxy)
		: Tcp::TcpContext(socketProxy)
	{
		this->mSocket = socketProxy;
	}

	bool TelnetClientContext::StartConnect()
	{
		return this->ConnectSync();
	}

	bool TelnetClientContext::ReadCommand(std::string& command)
	{
		std::istream is(&this->mRecvBuffer);
		if(this->RecvLineSync() <= 0)
		{
			return false;
		}
		std::getline(is, command);
		return true;
	}

	bool TelnetClientContext::SendCommand(const std::string& command)
	{
		return true;
	}
}

