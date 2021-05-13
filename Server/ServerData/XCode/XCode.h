#pragma once
enum XCode
{
	Successful = 0, 	 //成功
	Failure,	 //失败
	ParseMessageError,	//解析protobuf失败
	CallFunctionNotExist,	//调用方法不存在
	TimeoutAutoCall,		//超时自动调用
	CallLuaFunctionFail,	//调用lua方法失败
	JsonCastProtocbufFail,	//json转protocbuf失败
	ProtocbufCastJsonFail,	//protocbuf转json失败
	LuaCoroutineReturn,			//启动lua协程lua自己返回
	FunctionNameFail,
	CreatePorotbufFail,	//创建pb失败
	SerializationFailure, //序列化失败
	CommandArgsError,	//CM命令参数错误
	SessionIsNull,		//session是空
	SendMessageFail,	//发送消息失败
	NoCoroutineContext,	//没有协程上下文

	ParseJsonFailure,

	MysqlNotInCoroutine,	//不在协程中
	MysqlStartTaskFail,		//启动失败
	MysqlSocketIsNull,		//socket空
	MysqlSelectDbFailure,	//选择db失败
	MysqlInvokeFailure,		//执行sql语句失败


	RedisNotInCoroutine,
	RedisStartTaskFail,
	RedisSocketIsNull,
	RedisInvokeFailure,
	RedisJsonParseFail,

};