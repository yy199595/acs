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
        Auth,       //验证身份消息
        Ping,       //
        Request,    //请求消息
        Response,   //请求的返回
        Forward,    //网关转发给客户端消息
        Broadcast,  //广播消息
        SubPublish, //发布订阅消息
        Logout,     //退出消息
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
