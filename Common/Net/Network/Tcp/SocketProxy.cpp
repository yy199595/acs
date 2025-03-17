#include"Socket.h"
#include"Util/Tools/String.h"
namespace tcp
{


	Socket::Socket(asio::io_service& thread)
		: mContext(thread), mIsClient(false)
	{
		this->mPort = 0;
		this->mIsClient = false;
		this->MakeNewSocket();
	}
#ifdef __ENABLE_OPEN_SSL__
	Socket::Socket(Asio::Context& io, asio::ssl::context& ssl)
		: mContext(io), mIsClient(false)
	{
		this->mPort = 0;
		this->MakeNewSocket();
#ifdef __ENABLE_OPEN_SSL__
		this->mSslSocket = std::make_unique<Asio::ssl::Socket>(*this->mSocket, ssl);
#endif
	}
#endif


	bool Socket::Init()
    {
        asio::error_code code;
        const Asio::EndPoint endPoint = this->mSocket->remote_endpoint(code);
        if (!this->mSocket->is_open() || code.value() != Asio::OK)
        {
			return true;
        }
		this->mIsClient = false;
		this->mPort = endPoint.port();
		this->mIp = endPoint.address().to_string();
		this->mAddress = fmt::format("{0}:{1}", this->mIp, this->mPort);
		return true;
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
			this->Close();
		}
		this->mSocket = std::make_unique<Asio::Socket>(this->mContext);
		{
			this->SetOption(tcp::OptionType::NoDelay, true);
			this->SetOption(tcp::OptionType::KeepAlive, true);
		}
	}

	bool Socket::Init(const std::string& address)
	{
		std::string ip;
		unsigned short port = 0;
		if(help::Str::SplitAddr(address, ip, port))
		{
			return false;
		}
		return this->Init(ip, port);
	}

    bool Socket::Init(const std::string &ip, unsigned short port)
	{
		if (ip.empty() || port == 0)
		{
			return false;
		}
		this->mIp = ip;
		this->mPort = port;
		this->mIsClient = true;
		this->mAddress = fmt::format("{0}:{1}", ip, port);
		return true;
	}

	bool Socket::SetOption(tcp::OptionType type, bool val)
	{
		try
		{
			switch (type)
			{
				case tcp::OptionType::NoDelay:
					this->mSocket->set_option(asio::ip::tcp::no_delay(val));
					return true;
				case tcp::OptionType::KeepAlive:
					this->mSocket->set_option(asio::ip::tcp::socket::keep_alive(val)); //保持连接
					return true;
				case tcp::OptionType::CloseLinger:
					this->mSocket->set_option(asio::socket_base::linger(val, 0)); //立即关闭
					return true;
				case tcp::OptionType::ReuseAddress:
					this->mSocket->set_option(asio::ip::tcp::socket::reuse_address(val)); //重用地址
					return true;
			}
			return true;
		}
		catch(const std::system_error &)
		{
			return false;
		}
	}


	void Socket::Close()
	{
		try
		{
			this->mSocket->cancel();
			this->mSocket->shutdown(asio::socket_base::shutdown_both);
			this->mSocket->close();

#ifdef __ENABLE_OPEN_SSL__
			if (this->mSslSocket != nullptr)
			{
				this->mSslSocket->shutdown();
			}
#endif
		}
		catch (const std::system_error & err)
		{

		}
	}

	void Socket::Destroy()
	{
		this->Close();
		delete this;
	}
}
