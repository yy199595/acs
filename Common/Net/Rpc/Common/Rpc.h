//
// Created by zmhy0073 on 2021/10/14.
//

#pragma once

#include"Log/Common/CommonLogDef.h"
namespace rpc
{

	//消息类型
    namespace Type
    {
        constexpr char None = 0;
		constexpr char Auth = 1;       //验证身份消息
		constexpr char Ping = 2;       //
		constexpr char Pong = 3;       //
		constexpr char Request = 4;    //请求消息
		constexpr char Response = 5;   //请求的返回
		constexpr char Forward = 6;    //内网通过网关转发到客户端的消息
		constexpr char Broadcast = 7;  //内网广播到网关的消息
		constexpr char SubPublish = 8; //发布订阅消息
		constexpr char Logout = 9;     //退出消息
		constexpr char Client = 10; 	  //服务器发到客户端的消息
		// 自定义扩展
        constexpr char Max = 127;
    };

	namespace msg //协议格式
	{
		constexpr char none = 0;
		constexpr char bin = 1;  //带长度
		constexpr char json = 2;
		constexpr char text = 3; //不带长度
	}


	// rpc消息头
	namespace Header
	{
		constexpr const char* func = "$func";
		constexpr const char* code = "$code";
		constexpr const char* sock_id = "$sock";
		constexpr const char* client_sock_id = "$cli";
		constexpr const char* forward_tar = "$tar";
		constexpr const char* app_id = "$aid";
		constexpr const char * from_addr = "$from_addr";
		constexpr const char * id = "$id"; //actor id
		constexpr const char * channel = "$channel"; //频道
	}

	//协议类型
    namespace Proto
    {
		constexpr char None = 0;
		constexpr char Json = 1;
		constexpr char String = 2;
		constexpr char Protobuf = 3;
		constexpr char Lua = 4;
		// 自定义扩展
		constexpr char Max = std::numeric_limits<char>::max();
    };

	//网络类型
	namespace Net
	{
		constexpr char Tcp = 1; //tcp
		constexpr char Http = 2; //http
		constexpr char Redis = 3; //redis队列
		constexpr char Client = 4; //客户端专用
		constexpr char Udp = 5; //upd
		constexpr char Kcp = 6; //upd
		constexpr char Forward = 7;
		constexpr char Ws = 8;
		// 自定义扩展
		constexpr char Max = std::numeric_limits<char>::max();
	}

	//网关转发方式
	namespace Forward
	{
		constexpr char Fixed = 0; //固定转发
		constexpr char Random = 1; //随机分配
		constexpr char Hash = 2;  //id哈希
		constexpr char Next = 3; //轮询
		// 自定义扩展
		constexpr char Max = std::numeric_limits<char>::max();
	}

	//消息源
	namespace Source
	{
		constexpr char None = 0;
		constexpr char Client = 1;
		constexpr char Server = 2;
	}
	typedef unsigned short ProtoLenType;
//pragma pack(1)
	//协议头
	struct ProtoHead
	{
	public:
		ProtoLenType Len = 0; //协议包长度
		char Type = rpc::Type::None; //协议类型
		char Porto = rpc::Proto::None; //使用的通信协议
		char Source = rpc::Source::None; //消息源
		int RpcId = 0; // rpcId
	};

	constexpr int OuterBufferMaxSize = 4096; //外网数据包大小限制
	constexpr int RPC_PACK_HEAD_LEN = sizeof(ProtoLenType) + sizeof(char) * 3 + sizeof(int);
	constexpr int InnerBufferMaxSize = std::numeric_limits<unsigned short>::max() - sizeof(rpc::ProtoHead);
//#pragma pack()
}

// 消息长度 类型 协议 rpcid

