#pragma once

#ifdef _MSC_VER
#ifndef _WIN32_WINNT||_WIN32_WINNT!=0x0601
#define _WIN32_WINNT 0x0601 
#endif
#endif

#include<map>
#include<set>
#include<list>
#include<queue>
#include<vector>
#include<memory>
#include<unordered_map>
#include"asio.hpp"

using namespace std;


typedef asio::ip::udp AsioUdp;
typedef asio::ip::tcp AsioTcp;
typedef asio::io_service AsioContext;
typedef asio::ip::address AsioAddress;
typedef asio::io_service::work AsioWork;
typedef asio::ip::tcp::socket AsioTcpSocket;
typedef asio::ip::tcp::acceptor AsioTcpAcceptor;
typedef asio::ip::tcp::endpoint AsioTcpEndPoint;
typedef asio::ip::udp::socket AsioUdpSocket;
typedef asio::ip::udp::endpoint AsioUdpEndPoint;
typedef asio::ip::address_v4 AsioAddressV4;

typedef std::shared_ptr<std::string> SharedMessage;



namespace Sentry
{
	template<typename T>
	struct GKTypeProxy { typedef T Type; };
	template<typename T>
	struct GKTypeProxy<T *> { typedef T Type; };

	template<typename T>
	struct GKTypeProxy<T &> { typedef T Type; };

	template<typename T>
	struct GKTypeProxy<std::shared_ptr<T>> { typedef T Type; };
}

namespace Sentry
{
    template<typename T>
    class LocalObject
    {
    public:
        explicit LocalObject(const T * t) : mData(t) {}
        ~LocalObject() { delete this->mData;}
        void Cancel() { this->mData = nullptr; }
    private:
        const T * mData;
    };
}