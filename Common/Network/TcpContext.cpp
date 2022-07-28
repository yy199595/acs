//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpContext.h"
#include"Util/TimeHelper.h"
#include"Util/StringHelper.h"
namespace Tcp
{
	TcpContext::TcpContext(std::shared_ptr<SocketProxy> socket, size_t count)
		: mMaxCount(count), mRecvBuffer(count)
	{
		this->mSocket = socket;
		this->mLastOperTime = 0;
		this->mConnectCount = 0;
	}

    bool TcpContext::Reset(std::shared_ptr<SocketProxy> socket)
    {
        if(this->mSocket == nullptr)
        {
            this->mSocket = socket;
            this->mLastOperTime = 0;
            this->mConnectCount = 0;
            return true;
        }
        return false;
    }

	TcpContext::~TcpContext()
	{

	}


	void TcpContext::Connect()
	{
		this->mConnectCount++;
		assert(this->mSocket->IsRemote());
		unsigned short port = this->mSocket->GetPort();
		const std::string& ip = this->mSocket->GetIp();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		auto address = asio::ip::make_address_v4(ip);
		asio::ip::tcp::endpoint endPoint(address, port);
		std::shared_ptr<TcpContext> self = this->shared_from_this();

		printf("start connect %s\n" , this->mSocket->GetAddress().c_str());

		asio::error_code code;
		tcpSocket.close(code);
		tcpSocket.async_connect(endPoint, [this, self]
			(const asio::error_code& code)
		{
			if(!code)
			{
				this->mConnectCount = 0;
			}
			this->OnConnect(code, this->mConnectCount);
		});
		this->mLastOperTime = Helper::Time::GetNowSecTime();
	}

    void TcpContext::ReceiveLine()
	{
        this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read_until(tcpSocket, this->mRecvBuffer, "\r\n",
			[this, self](const asio::error_code& code, size_t size)
			{
                std::istream is(&this->mRecvBuffer);
                this->OnReceiveLine(code, is);
			});
	}

	void TcpContext::ReceiveSomeMessage()
	{
        if(this->mRecvBuffer.size() > 0)
        {
            asio::error_code code;
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is);
            return;
        }
        asio::error_code code;
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();

        if(tcpSocket.available(code) <= 0)
        {
            code = asio::error::eof;
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is);
            return;
        }
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer,
                         asio::transfer_at_least(1),
			[this, self](const asio::error_code& code, size_t size)
			{
                std::istream is(&this->mRecvBuffer);
				this->OnReceiveMessage(code, is);
			});
	}

    void TcpContext::ReceiveLength()
    {
        int length = sizeof(int);
        if (this->mRecvBuffer.size() >= length)
        {
            asio::error_code code;
            std::istream is(&this->mRecvBuffer);
            is.readsome((char *) &length, sizeof(int));
            this->OnReceiveLength(code, length);
            return;
        }
        else
        {
            length -= this->mRecvBuffer.size();
            this->mLastOperTime = Helper::Time::GetNowSecTime();
            AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
            std::shared_ptr<TcpContext> self = this->shared_from_this();
            asio::async_read(tcpSocket, this->mRecvBuffer,
                             asio::transfer_exactly(length),
                             [this, self](const asio::error_code &code, size_t size)
                             {
                                 char buffer[sizeof(int)] = { 0};
                                 size_t size1 = this->mRecvBuffer.size();
                                 std::istream is(&this->mRecvBuffer);
                                 size_t len = is.readsome(buffer, sizeof(int));
                                 if(len == sizeof(int))
                                 {
                                     int length = 0;
                                     memcpy(&length, buffer, sizeof(int));
                                     this->OnReceiveLength(code, length);
                                 }
                                 else
                                 {
                                     asio::error_code code;
                                     this->OnReceiveLength(code, 0);
                                 }
                             });
        }
    }

	void TcpContext::ReceiveMessage(int length)
	{
		assert(length > 0);
        if (length >= this->mMaxCount)
		{
			asio::error_code code =
                    std::make_error_code(std::errc::bad_message);
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is);
			return;
		}
		if (this->mRecvBuffer.size() >= length)
		{
			asio::error_code code;
            std::istream is(&this->mRecvBuffer);
            this->OnReceiveMessage(code, is);
			return;
		}
		length -= this->mRecvBuffer.size();
		this->mLastOperTime = Helper::Time::GetNowSecTime();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<TcpContext> self = this->shared_from_this();
		asio::async_read(tcpSocket, this->mRecvBuffer,
                         asio::transfer_exactly(length),
			[this, self](const asio::error_code& code, size_t size)
			{
                std::istream is(&this->mRecvBuffer);
                this->OnReceiveMessage(code, is);
			});
	}

	void TcpContext::Send(std::shared_ptr<ProtoMessage> message)
	{
        if(this->mMessagqQueue.empty())
        {
            this->mMessagqQueue.emplace_back(message);
            this->SendFromMessageQueue();
            return;
        }
        this->mMessagqQueue.emplace_back(message);
	}

    void TcpContext::SendFromMessageQueue()
    {
        assert(this->mSendBuffer.size() == 0);
        if(!this->mMessagqQueue.empty())
        {
            std::ostream os(&this->mSendBuffer);
            AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
            std::shared_ptr<TcpContext> self = this->shared_from_this();
            const int length = this->mMessagqQueue.front()->Serailize(os);
            asio::async_write(tcpSocket, this->mSendBuffer, [this, self]
                    (const asio::error_code& code, size_t size)
            {
                std::shared_ptr<Tcp::ProtoMessage> message = this->mMessagqQueue.front();
                if(!code)
                {
//                    if(length > 0)
//                    {
//                        this->SendFromMessageQueue();
//                        return;
//                    }
                    this->mMessagqQueue.pop_front();
                }
                this->ClearSendStream();
                this->OnSendMessage(code, message);
            });
            this->mLastOperTime = Helper::Time::GetNowSecTime();
        }
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
		asio::error_code code;
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		auto address = asio::ip::make_address_v4(this->mSocket->GetIp());
		asio::ip::tcp::endpoint endPoint(address, this->mSocket->GetPort());
		tcpSocket.connect(endPoint, code);
		if(code)
		{
			printf("sync connect [%s] failure", this->mSocket->GetAddress().c_str());
			return false;
		}
		return true;
	}

	int TcpContext::RecvSync(int length)
	{
		asio::error_code code;
        if(this->mRecvBuffer.size() >= length)
		{
			return length;
		}
		AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		int size = asio::read(tcpSocket, this->mRecvBuffer,
			asio::transfer_exactly(length), code);
		return code ? 0 : size;
	}

	int TcpContext::RecvLineSync()
	{
		asio::error_code code;
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
		return asio::read_until(tcpSocket, this->mRecvBuffer, "\r\n", code);
	}

	int TcpContext::SendSync(std::shared_ptr<ProtoMessage> message)
	{
		int sum = 0;
		asio::error_code code;
        std::ostream os(&this->mSendBuffer);
		int length = message->Serailize(os);
		AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
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