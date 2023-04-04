#pragma once

#include<istream>

#include"Network/Tcp/TcpContext.h"
#include"Async/Source/TaskSource.h"

namespace Tendo
{
	class TelnetProto;
	class ConsoleComponent;
}

namespace Tcp
{
	class TelnetClientContext : public Tcp::TcpContext
	{
	 public:
		explicit TelnetClientContext(const std::shared_ptr<SocketProxy>& socketProxy);
		~TelnetClientContext() = default;
	 public:
		bool StartConnect();
		bool ReadCommand(std::string & command);
		bool SendCommand(const std::string & command);
	};
}