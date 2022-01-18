//
// Created by yjz on 2022/1/19.
//
#include"HttpAsyncClient.h"

namespace GameKeeper
{
    HttpAsyncClient::HttpAsyncClient(std::shared_ptr<SocketProxy> socketProxy)
    {
        this->mSocket = socketProxy;
    }

    TaskSource<bool> HttpAsyncClient::ConnectAsync(const std::string *url)
    {
        
    }
}
