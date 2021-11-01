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
#include"ThreadQueue.h"
#include<asio.hpp>

using namespace std;

typedef int int32;
typedef long long int64;
typedef unsigned int uint32;
typedef float float32;
typedef double float64;
typedef short int16;
typedef unsigned short uint16;


typedef asio::ip::udp AsioUdp;
typedef asio::ip::tcp AsioTcp;
typedef asio::io_service AsioContext;
typedef asio::ip::address AsioAddress;
typedef asio::error_code AsioErrorCode;
typedef asio::io_service::work AsioWork;
typedef asio::ip::tcp::socket AsioTcpSocket;
typedef asio::ip::tcp::acceptor AsioTcpAcceptor;
typedef asio::ip::tcp::endpoint AsioTcpEndPoint;
typedef asio::ip::udp::socket AsioUdpSocket;
typedef asio::ip::udp::endpoint AsioUdpEndPoint;
typedef asio::ip::address_v4 AsioAddressV4;

typedef std::function<void(const AsioErrorCode &)> AcceptCallBack;
typedef std::function<void(const AsioErrorCode &)> AsioConnectCallBack;
typedef std::function<void(const AsioErrorCode &, const size_t)> AsioSendCallBack;
typedef std::function<void(const AsioErrorCode &, const size_t)> AsioReceiveCallBack;

typedef std::shared_ptr<AsioTcpSocket> SharedTcpSocket;

typedef std::shared_ptr<std::string> SharedMessage;



namespace GameKeeper
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

namespace GameKeeper
{
    template<typename T>
    class LocalObject
    {
    public:
        LocalObject(const T * t) : mData(t) {}
        ~LocalObject() { delete this->mData;}
    private:
        const T * mData;
    };
}