#pragma once;
enum XCode
{
	Successful = 0,//成功
	Failure = 1,//失败
	ParseMessageError = 2,//解析protobuf失败
	CallFunctionNotExist = 3,//调用方法不存在
	TimeoutAutoCall = 4,//超时自动调用
	CallLuaFunctionFail = 5,//调用lua方法错误
	JsonCastProtocbufFail = 6,//json转protobuf失败
	ProtocbufCastJsonFail = 7,//protobuf转json失败
	CreatePorotbufFail = 8,//创建protobuf失败
	SerializationFailure = 9,//序列化失败
	CommandArgsError = 10,//GM命令参数错误
	SessionIsNull = 11,//Session是空
	SendMessageFail = 12,//发送消息错误
	NoCoroutineContext = 13,//没有协程上下文
	ParseJsonFailure = 14,//解析json失败
	CacheMessageNextSend = 15,//缓存消息下次发送
	MysqlNotInCoroutine = 16,//Mysql没有协程上下文
	MysqlStartTaskFail = 17,//Mysql任务启动失败
	MysqlInitTaskFail = 18,//Mysql初始化失败
	MysqlSocketIsNull = 19,//Mysql连接是空
	MysqlSelectDbFailure = 20,//Mysql选择数据库失败
	MysqlInvokeFailure = 21,//Mysql执行sql语句失败
	RedisNotInCoroutine = 22,//Redis没有协程上下文
	RedisStartTaskFail = 23,//Redis任务启动失败
	RedisSocketIsNull = 24,//Redis连接是空
	RedisReplyIsNull = 25,//Redis回复是空
	RedisInvokeFailure = 26,//Rsedis命令执行失败
	AccountAlreadyExists = 27,//账号已经存在
	AccountNotExists = 28//账号不存在
};