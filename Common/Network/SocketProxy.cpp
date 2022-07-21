#include"SocketProxy.h"
#include<Util/Guid.h>
#include<spdlog/fmt/fmt.h>
namespace Sentry
{
	SocketProxy::SocketProxy(IAsioThread& thread)
		: mNetThread(thread)
	{
		this->mIsRemote = true;
		this->mSocket = new AsioTcpSocket(this->mNetThread);
	}

    void SocketProxy::Init()
    {
        this->mIsRemote = false;
        asio::error_code code;
        auto endPoint = this->mSocket->remote_endpoint(code);
        if (this->mSocket->is_open() && !code)
        {
            this->mPort = endPoint.port();
            this->mIp = endPoint.address().to_string();
            this->mAddress = fmt::format("{0}:{1}", this->mIp, this->mPort);
        }
    }

    SocketProxy::~SocketProxy()
    {
        delete this->mSocket;
    }

	SocketProxy::SocketProxy(IAsioThread& thread, const std::string& ip, unsigned short port)
			: mNetThread(thread)
	{
		this->mIp = ip;
		this->mPort = port;
		this->mIsRemote = true;
		assert(!this->mIp.empty() && this->mPort > 0);
		this->mAddress = fmt::format("{0}:{1}", ip, port);
		this->mSocket = new AsioTcpSocket(this->mNetThread);
	}

	void SocketProxy::Close()
	{
		if (this->mSocket->is_open())
		{
			asio::error_code code;
			this->mSocket->close(code);
		}
		asio::error_code code;
		this->mSocket->cancel(code);
	}
}
