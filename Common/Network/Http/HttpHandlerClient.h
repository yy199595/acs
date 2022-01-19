//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H
#include"HttpAsyncRequest.h"
#include"Network/SocketProxy.h"
namespace GameKeeper
{
    class HttpHandlerClient
    {
    public:
        HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy);
    public:
        std::shared_ptr<HttpHandlerRequest> ReadHandlerContent();
    private:
        std::shared_ptr<SocketProxy> mSocket;
    };
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
