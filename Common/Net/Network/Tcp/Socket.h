#pragma once
#include<mutex>
#include"Asio.h"
#include"Log/Common/CommonLogDef.h"

#ifdef __ENABLE_OPEN_SSL__
#include"asio/ssl.hpp"
#endif

namespace tcp
{
	 enum class OptionType
	 {
		 KeepAlive,
		 NoDelay,	//发包不延迟
		 CloseLinger,
		 ReuseAddress,
	 };

	 struct AddrInfo
	 {
	 public:

	 };

	class Socket
	{
	 public:
		explicit Socket(Asio::Context & io);
#ifdef __ENABLE_OPEN_SSL__
		explicit Socket(Asio::Context & io, asio::ssl::context & ssl);
#endif
		~Socket() = default;
	 public:
        void Init();
		void MakeNewSocket();
		bool CanRecvCount(size_t & count);
		void Init(const std::string & address);
		void Init(const std::string & ip, unsigned short port);
		inline Asio::Socket & Get() { return *this->mSocket; }
		inline bool IsOpen() const { return this->mSocket && this->mSocket->is_open(); }
	public:
		inline bool IsOpenSsl() const { return this->mIsOpenSsl; }
#ifdef __ENABLE_OPEN_SSL__
		inline Asio::ssl::Socket & SslSocket() { return *this->mSslSocket; }
#endif
	public:
		bool SetOption(OptionType type, bool val);
    public:
        void Close();
		inline std::string & GetIp() { return this->mIp;}
		unsigned short GetPort() const { return this->mPort;}
		inline bool IsClient() const { return this->mIsClient; }
		inline const std::string& GetAddress() { return this->mAddress; }
		void Clear() { this->mPort = 0; this->mAddress.clear(); this->mIp.clear(); }
	private:
		bool mIsClient;
		std::string mIp;
		bool mIsOpenSsl;
		unsigned short mPort;
		std::string mAddress;
		Asio::Context & mContext;
#ifdef __ENABLE_OPEN_SSL__
		std::unique_ptr<Asio::ssl::Socket> mSslSocket;
#endif
		std::unique_ptr<Asio::Socket> mSocket;
    };
}