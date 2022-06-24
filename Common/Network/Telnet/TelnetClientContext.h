#pragma once

#include<istream>
#include<Async/TaskSource.h>
#include<Network/SocketProxy.h>
#include"Network/TcpContext.h"
#include"DB/Mongo/MongoProto.h"
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
		void OnReceiveLine(const asio::error_code &code, asio::streambuf &buffer) final;
		void OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer) final;
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	 private:
		ConsoleComponent * mConsoleComponent;
		std::shared_ptr<Mongo::MongoQueryRequest> mRequest;
	};
}