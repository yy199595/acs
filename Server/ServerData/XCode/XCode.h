#pragma once
enum XCode
{
	Successful = 0, 	 //成功
	Failure = 1,	 //失败
	ParseMessageError = 2,	//解析protobuf失败
	CallFunctionNotExist = 3,	//调用方法不存在
	TimeoutAutoCall = 4,		//超时自动调用
	CallLuaFunctionFail = 5,	//调用lua方法失败
	JsonCastProtocbuferFail = 6,	//json转protocbuf失败
	FunctionNameFail=7,
	CreatePorotbufFail = 8,	//创建pb失败
	SerializationFailure = 9, //序列化失败
	ServerNotRegister = 10,		//服务器没注册
	AssestNotExist = 11,   //资源不存在
	WaitDownLoadAssest = 12,	//等待下载资源
	CommandArgsError = 13,	//CM命令参数错误
	SessionIsNull = 14,		//session是空
	SendMessageFail = 15,	//发送消息失败
};