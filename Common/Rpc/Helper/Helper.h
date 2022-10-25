//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_HELPER_H
#define APP_HELPER_H
#include"Client/Message.h"
namespace Sentry
{
    namespace Helper
    {
        std::shared_ptr<Rpc::Packet> MakeRpcPacket(const std::string & fullName);
        std::shared_ptr<Rpc::Packet> MakeRpcPacket(const std::string & service, const std::string & func);
    }
}


#endif //APP_HELPER_H
