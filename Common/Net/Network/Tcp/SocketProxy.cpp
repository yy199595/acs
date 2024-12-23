#include"Socket.h"
#include"Util/Tools/String.h"
namespace tcp
{


	Socket::Socket(asio::io_service& thread)
		: mContext(thread), mIsOpenSsl(false), mIsClient(false)
	{
		this->mPort = 0;
		this->mIsClient = false;
		this->MakeNewSocket();
	}
#ifdef __ENABLE_OPEN_SSL__
	Socket::Socket(Asio::Context& io, asio::ssl::context& ssl)
		: mContext(io), mIsOpenSsl(true), mIsClient(false)
	{
		this->mPort = 0;
		this->MakeNewSocket();
#ifdef __ENABLE_OPEN_SSL__
		if(this->mIsOpenSsl)
		{
			this->mSslSocket = std::make_unique<Asio::ssl::Socket>(*this->mSocket, ssl);
		}
#endif
	}
#endif

    void Socket::Init()
    {
        asio::error_code code;
        const Asio::EndPoint endPoint = this->mSocket->remote_endpoint(code);
        if (this->mSocket->is_open() && !code)
        {
			this->mIsClient = false;
            this->mPort = endPoint.port();
            this->mIp = endPoint.address().to_string();
            this->mAddress = fmt::format("{0}:{1}", this->mIp, this->mPort);
        }
    }

#ifdef __MEMORY_POOL_OPERATOR__
	std::mutex Socket::sMutex;
	std::vector<void *> Socket::sAllocArray;
	void* Socket::operator new(size_t size)
	{
		std::lock_guard<std::mutex> lock(sMutex);
		if(!sAllocArray.empty())
		{
			void * ptr = sAllocArray.back();
			sAllocArray.pop_back();
			return ptr;
		}
		return std::malloc(size);
	}

	void Socket::operator delete(void* ptr)
	{
		std::lock_guard<std::mutex> lock(sMutex);
		if(sAllocArray.size() >= 100)
		{
			std::free(ptr);
			return;
		}
		sAllocArray.emplace_back(ptr);
	}
#endif

	bool Socket::CanRecvCount(size_t& count)
	{
		Asio::Code code;
		count = this->mSocket->available(code);
		return code.value() == Asio::OK;
	}

	void Socket::MakeNewSocket()
	{
		if(this->mSocket && this->mSocket->is_open())
		{
			asio::error_code code;
			{
				this->mSocket->cancel(code);
				this->mSocket->shutdown(asio::socket_base::shutdown_both, code);
				this->mSocket->close(code);
			}
		}
		this->mSocket = std::make_unique<Asio::Socket>(this->mContext);
		this->SetOption(tcp::OptionType::NoDelay, true);
		this->SetOption(tcp::OptionType::KeepAlive, true);
	}

	void Socket::Init(const std::string& address)
	{
		std::string ip;
		unsigned short port = 0;
		if(help::Str::SplitAddr(address, ip, port))
		{
			this->Init(ip, port);
		}
	}

    void Socket::Init(const std::string &ip, unsigned short port)
	{
		if (ip.empty() || port == 0)
		{
			CONSOLE_LOG_ERROR("Inti Socket Error Address Error")
			return;
		}
		this->mIp = ip;
		this->mPort = port;
		this->mIsClient = true;
		this->mAddress = fmt::format("{0}:{1}", ip, port);
	}

	bool Socket::SetOption(tcp::OptionType type, bool val)
	{

		asio::error_code code;
		switch (type)
		{
			case tcp::OptionType::NoDelay:
				this->mSocket->set_option(asio::ip::tcp::no_delay(val), code);
				return true;
			case tcp::OptionType::KeepAlive:
				this->mSocket->set_option(asio::ip::tcp::socket::keep_alive(val), code); //保持连接
				return true;
			case tcp::OptionType::CloseLinger:
				this->mSocket->set_option(asio::socket_base::linger(val, 0), code); //立即关闭
				return true;
			case tcp::OptionType::ReuseAddress:
				this->mSocket->set_option(asio::ip::tcp::socket::reuse_address(val), code); //重用地址
				return true;
		}
		return code.value() == Asio::OK;
	}


	void Socket::Close()
	{
		asio::error_code code;
		this->mSocket->cancel(code);
		this->mSocket->shutdown(asio::socket_base::shutdown_both, code);
		this->mSocket->close(code);

#ifdef __ENABLE_OPEN_SSL__
		if (this->mIsOpenSsl && this->mSslSocket)
		{
			this->mSslSocket->shutdown(code);
		}
#endif
	}

	void Socket::Destory()
	{
		this->Close();
		delete this;
	}
}
