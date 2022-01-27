#include"SocketProxy.h"
#include<Util/Guid.h>
#include<spdlog/fmt/fmt.h>
namespace Sentry
{
    SocketProxy::SocketProxy(IAsioThread &thread, const std::string &name, std::shared_ptr<AsioTcpSocket> socket)
        : mNetThread(thread), mName(name)
    {
        this->mSocket = socket;
        this->mIsOpen = socket->is_open();
        this->mSocketId = Helper::ThreadGuid::Create();

    }

    void SocketProxy::RefreshState()
    {
        asio::error_code code;
        this->mIsOpen = this->mSocket->is_open();
        auto endPoint = this->mSocket->remote_endpoint(code);
        if(this->mIsOpen && !code)
        {
            unsigned int port = endPoint.port();
            std::string ip = endPoint.address().to_string();
            this->mAddress = fmt::format("{0}:{1}", ip, port);
        }
    }

	SocketProxy::SocketProxy(IAsioThread & thread, const std::string & name)
		: mNetThread(thread), mName(name)
	{
        this->mIsOpen = false;
        this->mSocketId = Helper::ThreadGuid::Create();
        AsioContext & context = this->mNetThread.GetContext();
        this->mSocket = std::make_shared<AsioTcpSocket>(context);
	}
	
	void SocketProxy::Close()
	{
        if(this->mIsOpen)
        {
            this->mIsOpen = false;
            asio::error_code code;
            this->mSocket->close(code);
        }
        asio::error_code code;
        this->mSocket->cancel(code);
	}
}
