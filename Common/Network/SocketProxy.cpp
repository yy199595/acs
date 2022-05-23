#include"SocketProxy.h"
#include<Util/Guid.h>
#include<spdlog/fmt/fmt.h>
namespace Sentry
{
	SocketProxy::SocketProxy(IAsioThread& thread)
		: mNetThread(thread)
	{
		//AsioContext& context = this->mNetThread.GetContext();
		this->mSocket = std::make_shared<AsioTcpSocket>(this->mNetThread);
	}

	SocketProxy::SocketProxy(IAsioThread& thread, std::shared_ptr<AsioTcpSocket> socket)
		: mNetThread(thread)
	{
		asio::error_code code;
		this->mSocket = socket;
		auto endPoint = this->mSocket->remote_endpoint(code);
		if (this->mSocket->is_open() && !code)
		{
			this->mPort = endPoint.port();
			this->mIp = endPoint.address().to_string();
			this->mAddress = fmt::format("{0}:{1}", this->mIp, this->mPort);
		}
	}

	SocketProxy::SocketProxy(IAsioThread& thread, const std::string& ip, unsigned short port)
			: mNetThread(thread)
	{
		this->mIp = ip;
		this->mPort = port;
		assert(!this->mIp.empty() && this->mPort > 0);
		this->mAddress = fmt::format("{0}:{1}", ip, port);
		this->mSocket = std::make_shared<AsioTcpSocket>(this->mNetThread);
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
