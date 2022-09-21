#pragma once

#include<istream>
#include"Tcp/SocketProxy.h"
#include"Tcp/TcpContext.h"
#include"Source/TaskSource.h"

namespace Sentry
{
    class TelnetContent
    {
    public:
        bool IsOk;
        string Command;
        std::string Parameter;
    };
}

namespace Sentry
{
	class TelnetProto;
	class ConsoleComponent;
}

namespace Tcp
{
	class TelnetClientContext : public Tcp::TcpContext
	{
	 public:
		explicit TelnetClientContext(std::shared_ptr<SocketProxy> socketProxy, ConsoleComponent * component);
		~TelnetClientContext() = default;
	 public:
		void StartRead();
		void SendProtoMessage(std::shared_ptr<TelnetProto> message);
	 private:
		void CloseContext();
		void OnReceiveLine(const asio::error_code &code, std::istream & is, size_t) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	 private:
		ConsoleComponent * mConsoleComponent;
	};
}