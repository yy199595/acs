//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H
#include<stdexcept>
#include<unordered_map>
#include"Log/Common/CommonLogDef.h"
namespace Msg
{
    namespace Type
    {
        constexpr int None = 0;
		constexpr int Auth = 1;       //验证身份消息
		constexpr int Ping = 2;       //
		constexpr int Request = 3;    //请求消息
		constexpr int Response = 4;   //请求的返回
		constexpr int Forward = 5;    //转发消息
		constexpr int Broadcast = 6;  //广播消息
		constexpr int SubPublish = 7; //发布订阅消息
		constexpr int Logout = 8;     //退出消息
		constexpr int Client = 9; 	  //服务器发到客户端的消息
        constexpr int Max = 255;
    };
    namespace Porto
    {
		constexpr int None = 0;
		constexpr int Json = 1;
		constexpr int String = 2;
		constexpr int Protobuf = 3;
		constexpr int Bson = 4;
		constexpr int Max = 255;
    };

	namespace Net
	{
		constexpr int Tcp = 1; //tcp
		constexpr int Http = 2; //http
	}
}

constexpr int RPC_PACK_HEAD_LEN = sizeof(int) + sizeof(char) + sizeof(char);

#endif //GameKeeper_MESSAGESTREAM_H
