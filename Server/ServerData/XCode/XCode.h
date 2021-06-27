#pragma once
enum XCode
{
	Successful = 0,//成功
	Failure = 1,//失败
	ParseMessageError = 2,//解析protobuf失败
	CallServiceNotFound = 3,//调用服务未发现
	CallFunctionNotExist = 4,//调用方法不存在
	CallArgsError = 5,//调用参数错误
	TimeoutAutoCall = 6,//超时自动调用
	CallLuaFunctionFail = 7,//调用lua方法错误
	JsonCastProtocbufFail = 8,//json转protobuf失败
	ProtocbufCastJsonFail = 9,//protobuf转json失败
	CreatePorotbufFail = 10,//创建protobuf失败
	SerializationFailure = 11,//序列化失败
	CommandArgsError = 12,//GM命令参数错误
	SessionIsNull = 13,//Session是空
	SendMessageFail = 14,//发送消息错误
	NoCoroutineContext = 15,//没有协程上下文
	ParseJsonFailure = 16,//解析json失败
	CacheMessageNextSend = 17,//缓存消息下次发送
	MysqlNotInCoroutine = 18,//Mysql没有协程上下文
	MysqlStartTaskFail = 19,//Mysql任务启动失败
	MysqlInitTaskFail = 20,//Mysql初始化失败
	MysqlSocketIsNull = 21,//Mysql连接是空
	MysqlSelectDbFailure = 22,//Mysql选择数据库失败
	MysqlInvokeFailure = 23,//Mysql执行sql语句失败
	RedisNotInCoroutine = 24,//Redis没有协程上下文
	RedisStartTaskFail = 25,//Redis任务启动失败
	RedisSocketIsNull = 26,//Redis连接是空
	RedisReplyIsNull = 27,//Redis回复是空
	RedisInvokeFailure = 28,//Rsedis命令执行失败
	AccountAlreadyExists = 29,//账号已经存在
	AccountNotExists = 30//账号不存在
};