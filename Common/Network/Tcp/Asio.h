#include"asio.hpp"
namespace Asio
{
    typedef asio::error_code Code;
    typedef asio::io_service Context;
    typedef asio::ip::tcp::socket Socket;
    typedef asio::ip::tcp::endpoint EndPoint;
    typedef asio::io_service::work ContextWork;
    typedef asio::ip::address_v4 AddressV4;
    typedef asio::ip::tcp::resolver Resolver;
    typedef asio::ip::tcp::acceptor Acceptor;
    typedef asio::ip::tcp::resolver::query ResolverQuery;
}