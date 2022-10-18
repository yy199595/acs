//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include<stdexcept>
#include<unordered_map>
#include"Log/CommonLogDef.h"
namespace Tcp
{
    enum class Type
    {
        None,
        Auth,
        Ping,
        Request,
        Response,
        Forward,
        Broadcast,
        Max
    };
    enum class Porto
    {
        None,
        Json,
        String,
        Protobuf,
        Max
    };
}


typedef std::logic_error rpc_error;
constexpr int RPC_PACK_HEAD_LEN = sizeof(int) + sizeof(char) + sizeof(char);

#endif //GameKeeper_MESSAGESTREAM_H
