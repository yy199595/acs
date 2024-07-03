//
// Created by zmhy0073 on 2021/10/14.
//

#ifndef GameKeeper_MESSAGESTREAM_H
#define GameKeeper_MESSAGESTREAM_H

#include"Log/Common/CommonLogDef.h"
namespace rpc
{
	//消息类型
    namespace Type
    {
        constexpr char None = 0;
		constexpr char Auth = 1;       //验证身份消息
		constexpr char Ping = 2;       //
		constexpr char Request = 3;    //请求消息
		constexpr char Response = 4;   //请求的返回
		constexpr char Forward = 5;    //内网通过网关转发到客户端的消息
		constexpr char Broadcast = 6;  //内网广播到网关的消息
		constexpr char SubPublish = 7; //发布订阅消息
		constexpr char Logout = 8;     //退出消息
		constexpr char Client = 9; 	  //服务器发到客户端的消息
        constexpr char Max = 127;
    };
	//协议类型
    namespace Porto
    {
		constexpr char None = 0;
		constexpr char Json = 1;
		constexpr char String = 2;
		constexpr char Protobuf = 3;
		constexpr char Bson = 4;
		constexpr char Max = 127;
    };

	//网络类型
	namespace Net
	{
		constexpr char Tcp = 1; //tcp
		constexpr char Http = 2; //http
		constexpr char Redis = 3; //redis队列
		constexpr char Client = 4; //客户端专用
		constexpr char Max = 127;
	}

	//网关转发方式
	namespace Forward
	{
		constexpr int Fixed = 0; //固定转发
		constexpr int Random = 1; //随机分配
		constexpr int Hash = 2;  //id哈希
	}

	//协议头
	struct ProtoHead
	{
	public:
		unsigned short Len = 0; //协议包长度
		char Type = rpc::Type::None; //协议类型
		char Porto = rpc::Porto::None; //使用的通信协议
		int RpcId = 0; // rpcId
	};

	constexpr int OuterBufferMaxSize = 4096; //外网数据包大小限制
	constexpr int RPC_PACK_HEAD_LEN = sizeof(ProtoHead);
	constexpr int InnerBufferMaxSize = std::numeric_limits<unsigned short>::max() - sizeof(rpc::ProtoHead);
}

// 消息长度 类型 协议 rpcid


#endif //GameKeeper_MESSAGESTREAM_H
