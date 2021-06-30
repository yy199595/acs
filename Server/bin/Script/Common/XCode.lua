XCode =
{
	Successful = 0,--成功
	Failure = 1,--失败
	ParseMessageError = 2,--解析protobuf失败
	CallServiceNotFound = 3,--调用服务未发现
	CallFunctionNotExist = 4,--调用方法不存在
	CallArgsError = 5,--调用参数错误
	TimeoutAutoCall = 6,--超时自动调用
	CallLuaFunctionFail = 7,--调用lua方法错误
	NotResponseMessage = 8,--不返回数据
	JsonCastProtocbufFail = 9,--json转protobuf失败
	ProtocbufCastJsonFail = 10,--protobuf转json失败
	CreatePorotbufFail = 11,--创建protobuf失败
	SerializationFailure = 12,--序列化失败
	CommandArgsError = 13,--GM命令参数错误
	SessionIsNull = 14,--Session是空
	SendMessageFail = 15,--发送消息错误
	NoCoroutineContext = 16,--没有协程上下文
	ParseJsonFailure = 17,--解析json失败
	CacheMessageNextSend = 18,--缓存消息下次发送
	MysqlNotInCoroutine = 19,--Mysql没有协程上下文
	MysqlStartTaskFail = 20,--Mysql任务启动失败
	MysqlInitTaskFail = 21,--Mysql初始化失败
	MysqlSocketIsNull = 22,--Mysql连接是空
	MysqlSelectDbFailure = 23,--Mysql选择数据库失败
	MysqlInvokeFailure = 24,--Mysql执行sql语句失败
	RedisNotInCoroutine = 25,--Redis没有协程上下文
	RedisStartTaskFail = 26,--Redis任务启动失败
	RedisSocketIsNull = 27,--Redis连接是空
	RedisReplyIsNull = 28,--Redis回复是空
	RedisInvokeFailure = 29,--Rsedis命令执行失败
	AccountAlreadyExists = 30,--账号已经存在
	AccountNotExists = 31--账号不存在
}
return XCode