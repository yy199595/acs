//
// Created by 64658 on 2024/10/28.
//

#include "KcpSession.h"

namespace kcp
{
	Session::Session(asio_udp::socket& sock, asio_udp::endpoint& endpoint)
			: mSocket(sock), mRemote(endpoint)
	{

	}

	void Session::Send(tcp::IProto* message)
	{
		Asio::Executor executor = this->mSocket.get_executor();
		asio::post(executor, [this, message]()
		{
			std::ostream stream(&this->mSendBuffer);
			int length = message->OnSendMessage(stream);
			this->mSocket.async_send_to(this->mSendBuffer.data(), this->mRemote,
					[this, length, message](const asio::error_code& code, size_t size)
					{
						if (code.value() != Asio::OK)
						{
							return;
						}
						this->mSendBuffer.consume(size);
						unsigned short port = this->mRemote.port();
						std::string ip = this->mRemote.address().to_string();
						//CONSOLE_LOG_ERROR("send:({}:{}) size:{}", ip, port, size);
					});
		});
	}
}