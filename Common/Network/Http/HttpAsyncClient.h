//
// Created by yjz on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPASYNCCLIENT_H
#define GAMEKEEPER_HTTPASYNCCLIENT_H
#include<SocketProxy.h>
#include<Async/TaskSource.h>
namespace GameKeeper
{
    class HttpAsyncClient : std::enable_shared_from_this<HttpAsyncClient>
    {
    public:
        HttpAsyncClient(std::shared_ptr<SocketProxy> socketProxy);

    public:
        TaskSource<bool> ConnectAsync(const std::string * url);
    private:
        std::shared_ptr<SocketProxy> mSocket;
    };
}
#endif //GAMEKEEPER_HTTPASYNCCLIENT_H
