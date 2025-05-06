#pragma once
#ifdef _MSC_VER
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif // !_WIN32_WINNT
#endif

#include"asio.hpp"
#ifdef __ENABLE_OPEN_SSL__
#include"asio/ssl.hpp"
#endif

namespace Asio
{
	constexpr int OK = 0;
    typedef asio::error_code Code;
    typedef asio::io_context Context;
    typedef asio::ip::tcp::socket Socket;
    typedef asio::ip::tcp::endpoint EndPoint;
    typedef asio::io_service::work ContextWork;
    typedef asio::ip::address_v4 AddressV4;
    typedef asio::ip::tcp::resolver Resolver;
    typedef asio::ip::tcp::acceptor Acceptor;
    typedef asio::ip::tcp::resolver::query ResolverQuery;
    typedef asio::ip::address Address;
    typedef asio::streambuf StreamBuffer;
    typedef asio::steady_timer Timer;
	typedef asio::ip::tcp::socket::executor_type Executor;
	namespace ssl
	{
#ifdef __ENABLE_OPEN_SSL__
		typedef asio::ssl::context Context ;
		typedef asio::ssl::stream<asio::ip::tcp::socket&> Socket;
#endif
	}
}