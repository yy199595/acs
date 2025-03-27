//
// Created by 64658 on 2024/10/28.
//

#include "KcpSession.h"
#include "Util/Tools/TimeHelper.h"
namespace kcp
{
	Session::Session(asio_udp::socket& sock, asio_udp::endpoint& endpoint)
			: mSocket(sock), mRemote(endpoint), mSendStream(&mSendBuffer)
	{
		this->mKcp = ikcp_create(0x01, this);
		this->mKcp->output = kcp::OnKcpSend;
		this->mLastTime = help::Time::NowSec();
		ikcp_wndsize(this->mKcp, kcp::BUFFER_COUNT, kcp::BUFFER_COUNT);
		ikcp_nodelay(this->mKcp, 1, kcp::REFRESH_INTERVAL, kcp::RESEND, 1);
		this->mAddress = fmt::format("{}:{}", endpoint.address().to_string(), endpoint.port());
	}

	void Session::Send(const char* buf, int len)
	{
		this->mSocket.send_to(asio::buffer(buf, len), this->mRemote);
	}

	int Session::Decode(const char* message, int len, char * buffer)
	{
		ikcp_input(this->mKcp, message, len);
		this->mLastTime = help::Time::NowSec();
		//LOG_DEBUG("[{}] receive message count={}", this->mAddress, len)
		return ikcp_recv(this->mKcp, buffer, kcp::BUFFER_COUNT);
	}

	void Session::Send(tcp::IProto* message)
	{
		message->OnSendMessage(this->mSendStream);

		int len = (int)this->mSendBuffer.size();
		const char* msg = asio::buffer_cast<const char*>(this->mSendBuffer.data());
		{
			ikcp_send(this->mKcp, msg, len);
			this->mSendBuffer.consume(len);
		}
		delete message;
	}

	void Session::Update(long long t)
	{
		ikcp_update(this->mKcp, (IUINT32)t);
	}
}