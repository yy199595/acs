#include"SocketProxy.h"
#include<Util/Guid.h>
#include<spdlog/fmt/fmt.h>
namespace Sentry
{
	SocketProxy::SocketProxy(IAsioThread& thread)
		: mNetThread(thread)
	{
		AsioContext& context = this->mNetThread.GetContext();
		this->mSocket = std::make_shared<AsioTcpSocket>(context);
	}

	SocketProxy::SocketProxy(IAsioThread& thread, std::shared_ptr<AsioTcpSocket> socket)
		: mNetThread(thread)
	{
		asio::error_code code;
		this->mSocket = socket;
		this->mIsOpen = socket->is_open();
		this->mSocketId = Helper::ThreadGuid::Create();
		auto endPoint = this->mSocket->remote_endpoint(code);
		if (this->mIsOpen && !code)
		{
			unsigned int port = endPoint.port();
			std::string ip = endPoint.address().to_string();
			this->mAddress = fmt::format("{0}:{1}", ip, port);
		}
	}

	SocketProxy::SocketProxy(IAsioThread& thread, const std::string& ip, unsigned short port)
			: mNetThread(thread)
	{
		this->mIsOpen = false;
		this->mSocketId = Helper::ThreadGuid::Create();
		AsioContext& context = this->mNetThread.GetContext();
		this->mSocket = std::make_shared<AsioTcpSocket>(context);
		this->mAddress = fmt::format("{0}:{1}", ip, port);
	}

	void SocketProxy::Close()
	{
		if (this->mIsOpen)
		{
			this->mIsOpen = false;
			asio::error_code code;
			this->mSocket->close(code);
		}
		asio::error_code code;
		this->mSocket->cancel(code);
	}
}
