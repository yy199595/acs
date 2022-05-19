//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include <Define/CommonLogDef.h>
#include <Define/CommonTypeDef.h>
#include <Protocol/com.pb.h>
#include <Protocol/c2s.pb.h>
#include <queue>

#define PROTO_POOL_COUNT 1024

enum class RPC_TYPE
{
	RPC_TYPE_REQUEST = 0x01, //服务器请求
	RPC_TYPE_RESPONSE = 0x02, //服务器返回
	RPC_TYPE_CALL_CLIENT = 0x03, //服务器调用客户端
	RPC_TYPE_CLIENT_REQUEST = 0x04, //客户端请求
};

#endif //GameKeeper_MESSAGESTREAM_H
