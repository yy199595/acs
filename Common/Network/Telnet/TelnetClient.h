#pragma once

#include<istream>
#include<Network/SocketProxy.h>
#include<Async/TaskSource.h>
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
	class ConsoleComponent;
	class TelnetClient
	{
	public:
		 explicit TelnetClient(std::shared_ptr<SocketProxy> socketProxy);
		~TelnetClient() = default;
    public:
        bool Response(const std::string & content);
        std::shared_ptr<TelnetContent> ReadCommand();
        bool IsOpen() { return this->mSocket->IsOpen(); }
        const std::string & GetAddress() { return this->mSocket->GetAddress();}
    private:
        void ResponseData(std::shared_ptr<TaskSource<bool>> taskSource);
        void ReadData(std::shared_ptr<TaskSource<bool>> taskSource, std::shared_ptr<TelnetContent> content);
    private:
        const std::string mDelim;
		asio::streambuf mSendBuffer;
        asio::streambuf mReceiveBuffer;
        std::shared_ptr<SocketProxy> mSocket;
	};
}