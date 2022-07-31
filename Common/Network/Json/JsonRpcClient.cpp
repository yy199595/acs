//
// Created by yjz on 22-7-31.
//

#include "JsonRpcClient.h"

namespace Tcp
{
    JsonRpcClient::JsonRpcClient(std::shared_ptr<SocketProxy> socket)
        : TcpContext(socket)
    {
        
    }
}