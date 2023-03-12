//
// Created by 64658 on 2023/3/12.
//
#include"Asio.h"
using asio::ip::tcp;
namespace Asio
{
    void GetNetCardAddressList(std::vector<std::string> & list)
    {
        asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(asio::ip::host_name(), "");
        tcp::resolver::iterator iter = resolver.resolve(query);
        tcp::resolver::iterator end; // End marker.
        while (iter != end)
        {
            tcp::endpoint ep = *iter++;
            list.emplace_back(ep.address().to_string());
        }
    }
}