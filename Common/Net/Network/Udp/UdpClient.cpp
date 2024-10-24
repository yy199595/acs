//
// Created by 64658 on 2024/10/24.
//

#include "UdpClient.h"
#include "Log/Common/CommonLogDef.h"
#include "Util/Tools/String.h"
namespace udp
{
	Client::Client(asio::any_io_executor & io)
		: mDelete(false) , mSocket(nullptr), mExecutor(io)
	{

	}

	Client::~Client()
	{
		if(this->mDelete) { delete this->mSocket; }
	}

	bool Client::Init(const std::string& local)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(local, ip, port))
		{
			return false;
		}
		try
		{
			this->mDelete = true;
			this->mEndPoint = asio_udp::endpoint(asio::ip::make_address(ip), port);
			this->mSocket = new asio_udp::socket(this->mExecutor, this->mEndPoint);
			return true;
		}
		catch (std::exception & e)
		{
			return false;
		}
	}

	bool Client::Init(asio_udp::socket* sock, const std::string& remote)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(remote, ip, port))
		{
			return false;
		}
		this->mSocket = sock;
		this->mDelete = false;
		this->mEndPoint = asio_udp::endpoint(asio::ip::make_address(ip), port);
		return true;
	}

	void Client::Send(tcp::IProto* message)
	{
		asio::post(this->mExecutor, [this, message]() {
			std::ostream stream(&this->mSendBuffer);
			int length = message->OnSendMessage(stream);
			CONSOLE_LOG_ERROR("{}:{}", this->mEndPoint.address().to_string(), this->mEndPoint.port());
			this->mSocket->async_send_to(this->mSendBuffer.data(),
					this->mEndPoint, [this](const asio::error_code & code, size_t size)
			{
				if(code.value() != Asio::OK)
				{

				}
				CONSOLE_LOG_ERROR("code:{} size:{}", code.message(), size);
			});
		});
	}
}