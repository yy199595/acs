//
// Created by yjz on 22-7-31.
//

#ifndef SERVER_JSONRPCCLIENT_H
#define SERVER_JSONRPCCLIENT_H
#include"Network/TcpContext.h"

namespace Tcp
{

    class JsonRpcClient : public TcpContext
    {
    public:
        JsonRpcClient(std::shared_ptr<SocketProxy> socket);
    };
}


#endif //SERVER_JSONRPCCLIENT_H
