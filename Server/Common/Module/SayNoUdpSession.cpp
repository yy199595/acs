#include"SayNoUdpSession.h"

namespace SoEasy
{
	SayNoUdpSession::SayNoUdpSession(SharedGameObject obj)
		: Component(obj)
	{
		this->mUdpSocket = nullptr;
		this->mIsReceiveMsg = false;
		this->mIsInitEndpoint = false;
		this->mPackageNumberof = 0;
	}

	void SayNoUdpSession::OnFrameStart()
	{
		
	}

	void SayNoUdpSession::StartReceiveMsg()
	{
		if (this->mUdpSocket && mIsReceiveMsg)
		{
			const size_t size = sizeof(unsigned int);
			this->mUdpSocket->async_receive_from(asio::buffer(this->udpReceiveBuffer, size), *mRemotePoint,
				[this](const AsioErrorCode & code, const size_t size)
			{
				size_t packageSize = 0; unsigned int packageNumberof = 0;
				memcpy(this->udpReceiveBuffer, &packageSize, sizeof(unsigned int));
				memcpy(this->udpReceiveBuffer + sizeof(udpReceiveBuffer), &packageNumberof, sizeof(unsigned int));
				this->ReceiveAllMessage(packageSize, packageNumberof);
			});
		}
	}

	void SayNoUdpSession::ReceiveAllMessage(const size_t size, const unsigned int packageNum)
	{
		if (this->mUdpSocket && mIsReceiveMsg)
		{
			this->mUdpSocket->async_receive_from(asio::buffer(this->udpReceiveBuffer, size), *mRemotePoint,
				[this](const AsioErrorCode & code, const size_t size)
			{
				if (!code)
				{
					SayNoDebugLog("receive udp msg size = " << size);
				}
			});
		}
	}

	void SayNoUdpSession::InitUdpSession(const std::string & ip, const unsigned short port, bool isReceive)
	{
		this->mIsReceiveMsg = isReceive;
		this->mIsInitEndpoint = true;
		this->mRemotePoint = new AsioUdpEndPoint(AsioAddress::from_string(ip), port);
	}

	void SayNoUdpSession::SendPackage(const std::string & message)
	{
		const char * msg = message.c_str();
		const size_t size = message.size();
		this->SendPackage(msg, size);
	}
	void SayNoUdpSession::SendPackage(const char * message, const size_t size)
	{
		SayNoAssertRet(this->mUdpSocket, "udp socket is null");
		this->ClearBuffer(this->udpSendMsgBuffer);
		const size_t packageSize = + sizeof(unsigned int) + size;

		memcpy(this->udpSendMsgBuffer, &size, sizeof(unsigned int));
		memcpy(this->udpSendMsgBuffer + sizeof(unsigned int), &this->mPackageNumberof, sizeof(unsigned int));
		memcpy(this->udpSendMsgBuffer + sizeof(unsigned int) + sizeof(unsigned int), message, size);
		this->mUdpSocket->async_send_to(asio::buffer(message, packageSize), *mRemotePoint,
			[](const asio::error_code & code, const size_t t)
		{
			if (code) return;
			//SayNoDebugWarning("send msg by udp size = " << t);
		});
	}

}
