//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpContext.h"
#include"Time/TimeHelper.h"
#include"String/StringHelper.h"
#include"Message/ProtoMessage.h"
namespace Tcp
{
	TcpContext::TcpContext(std::shared_ptr<SocketProxy> socket, size_t count)
		: mMaxCount(count), mRecvBuffer(count)
	{
        this->mSendCount = 0;
		this->mLastOperTime = 0;
		this->mConnectCount = 0;
        this->mSocket = std::move(socket);
    }

    bool TcpContext::Reset(std::shared_ptr<SocketProxy> socket)
    {
        if(this->mSocket != nullptr && this->mSocket->GetSocket().is_open())
        {
            return false;
        }
        this->mLastOperTime = 0;
        this->mConnectCount = 0;
        this->ClearSendStream();
        this->ClearRecvStream();
        this->mSocket = std::move(socket);
        return true;
    }


	void TcpContext::Connect()
	{
        Asio::Code code;
        this->mConnectCount++;
		assert(this->mSocket->IsRemote());
		unsigned short port = this->mSocket->GetPort();
		const std::string& ip = this->mSocket->GetIp();
        Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		Asio::AddressV4 address = asio::ip::make_address_v4(ip, code);
        if(code)
        {
            CONSOLE_LOG_ERROR("[" << ip << ":" << port << "] " << code.message());
            return;
        }
		Asio::EndPoint endPoint(address, port);
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		//printf("start connect %s\n" , this->mSocket->GetAddress().c_str());

		tcpSocket.close(code);
		tcpSocket.async_connect(endPoint, [this, self]
			(const Asio::Code & code)
		{
			if(!code)
			{
				this->mConnectCount = 0;
			}
			this->OnConnect(code, this->mConnectCount);
		});
		this->mLastOperTime = Helper::Time::NowSecTime();
	}

    void TcpContext::ReceiveLine()
	{
        this->mLastOperTime = Helper::Time::NowSecTime();
        Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read_until(tcpSocket, this->mRecvBuffer, "\r\n",
			[this, self](const Asio::Code & code, size_t size)
			{
                std::istream is(&this->mRecvBuffer);
                this->OnReceiveLine(code, is, size);
			});
	}

	void TcpContext::ReceiveSomeMessage()
	{
        if(this->mRecvBuffer.size() > 0)
        {
            Asio::Code code;
            size_t size = this->mRecvBuffer.size();
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is, size);
            return;
        }
        Asio::Code code;
		this->mLastOperTime = Helper::Time::NowSecTime();
		Asio::Socket & tcpSocket = this->mSocket->GetSocket();
        if(tcpSocket.available(code) <= 0)
        {
            code = asio::error::eof;
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is, 0);
            return;
        }
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer,
                         asio::transfer_at_least(1),
			[this, self](const asio::error_code& code, size_t size)
			{
                std::istream is(&this->mRecvBuffer);
				this->OnReceiveMessage(code, is, size);
			});
	}
    
	void TcpContext::ReceiveMessage(int length)
	{
		if(length <= 0)
		{
			CONSOLE_LOG_FATAL(length);
			return;
		}
        if (length >= this->mMaxCount)
		{
            std::istream is(&this->mRecvBuffer);
            Asio::Code code = std::make_error_code(std::errc::bad_message);
            this->OnReceiveMessage(code, is, length);
			return;
		}
		if (this->mRecvBuffer.size() >= length)
		{
			Asio::Code code;
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is, length);
			return;
		}
		length -= this->mRecvBuffer.size();
		this->mLastOperTime = Helper::Time::NowSecTime();
        Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer,
                         asio::transfer_exactly(length),
			[this, self](const asio::error_code& code, size_t size)
			{
                std::istream is(&this->mRecvBuffer);
                this->OnReceiveMessage(code, is, size);
			});
	}

	void TcpContext::Write(std::shared_ptr<ProtoMessage> message)
	{
        if(this->mMessagqQueue.empty())
        {
            this->mMessagqQueue.emplace_back(message);
            this->mSendCount = this->mMessagqQueue.size();
            this->SendFromMessageQueue();
            return;
        }
        this->mMessagqQueue.emplace_back(message);
        this->mSendCount = this->mMessagqQueue.size();
	}

    size_t TcpContext::PopAllMessage()
    {
        size_t count = 0;
        while(!this->mMessagqQueue.empty())
        {
            count++;
            this->mMessagqQueue.pop_front();
        }
        this->mSendCount = this->mMessagqQueue.size();
        return count;
    }

    std::shared_ptr<ProtoMessage> TcpContext::PopMessage()
    {
        std::shared_ptr<ProtoMessage> message;
        if(!this->mMessagqQueue.empty())
        {
            message = this->mMessagqQueue.front();
            this->mMessagqQueue.pop_front();
            this->mSendCount = this->mMessagqQueue.size();
        }
        return message;
    }

    bool TcpContext::SendFromMessageQueue()
    {
        //assert(this->mSendBuffer.size() == 0);
		//assert(this->mMessagqQueue.size() <= 100);
        if(!this->mMessagqQueue.empty())
        {
            std::ostream os(&this->mSendBuffer);
            Asio::Socket & tcpSocket = this->mSocket->GetSocket();
            std::shared_ptr<TcpContext> self = this->shared_from_this();
            const int length = this->mMessagqQueue.front()->Serailize(os);
            asio::async_write(tcpSocket, this->mSendBuffer, [this, self, length]
                    (const Asio::Code & code, size_t size)
            {
                if(length > 0 && !code)
                {
                    this->SendFromMessageQueue();
                    return;
                }
                this->ClearSendStream();
                this->OnSendMessage(code, this->mMessagqQueue.front());
            });
            this->mLastOperTime = Helper::Time::NowSecTime();
            return true;
        }
        return false;
    }
}

namespace Tcp
{
	void TcpContext::ClearRecvStream()
	{
        if(this->mRecvBuffer.size() > 0)
		{
			std::iostream os(&this->mRecvBuffer);
			os.ignore(this->mRecvBuffer.size());
        }
	}

	void TcpContext::ClearSendStream()
	{
        if(this->mSendBuffer.size() > 0)
		{
			std::iostream os(&this->mSendBuffer);
			os.ignore(this->mSendBuffer.size());
		}
	}

	bool TcpContext::ConnectSync()
	{
		Asio::Code code;
        Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		Asio::AddressV4 address = asio::ip::make_address_v4(this->mSocket->GetIp());
		Asio::EndPoint endPoint(address, this->mSocket->GetPort());
		tcpSocket.connect(endPoint, code);
		if(code)
		{
			tcpSocket.close(code);
			//printf("sync connect [%s] failure", this->mSocket->GetAddress().c_str());
			return false;
		}
		return true;
	}

    size_t TcpContext::RecvSync(int length)
	{
		Asio::Code code;
        if(this->mRecvBuffer.size() >= length)
		{
			return length;
		}
        Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		size_t size = asio::read(tcpSocket, this->mRecvBuffer,
			asio::transfer_exactly(length), code);
		return code ? 0 : size;
	}

	size_t TcpContext::RecvLineSync()
	{
		Asio::Code code;
        Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		return asio::read_until(tcpSocket, this->mRecvBuffer, "\r\n", code);
	}

	size_t TcpContext::SendSync(const std::shared_ptr<ProtoMessage>& message)
	{
		size_t sum = 0;
		Asio::Code code;
        std::ostream os(&this->mSendBuffer);
		int length = message->Serailize(os);
		Asio::Socket & tcpSocket = this->mSocket->GetSocket();
		sum += asio::write(tcpSocket, this->mSendBuffer, code);
		while(length > 0 && sum > 0 && !code)
		{
			try
			{
				sum += asio::write(tcpSocket, this->mSendBuffer);
				length = message->Serailize(os);
			}
			catch (std::system_error & error)
			{
				printf("sync send failure %s\n", error.what());
				return 0;
			}
		}
		return sum;
	}
}