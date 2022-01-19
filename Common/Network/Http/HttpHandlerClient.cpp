//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"

namespace GameKeeper
{
    HttpHandlerClient::HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy)
    {
        this->mSocket = socketProxy;
    }

    std::shared_ptr<HttpHandlerRequest> HttpHandlerClient::ReadHandlerContent()
    {
        return nullptr;
    }
}