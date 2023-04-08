#pragma once
namespace XCode 
{
	 constexpr int Successful = 0;//成功
	 constexpr int Failure = 0;//失败
	 constexpr int NetTimeout = 0;//网络超时
	 constexpr int NetReceiveFailure = 0;//接收网络消息失败
	 constexpr int NetSendFailure = 0;//发送网络消息失败
	 constexpr int NetConnectFailure = 0;//连接服务器失败
	 constexpr int NetActiveShutdown = 0;//主动关闭连接
	 constexpr int NetBigDataShutdown = 0;//数据包过大
	 constexpr int ParseMessageError = 0;//解析protobuf失败
	 constexpr int UnKnowPacket = 0;//恶意数据包
	 constexpr int ParseRequestDataError = 0;//解析请求数据包失败
	 constexpr int ParseResponseDataError = 0;//解析响应数据包失败
	 constexpr int CallServiceNotFound = 0;//调用服务未发现
	 constexpr int NotFoundRpcConfig = 0;//没有rpc配置
	 constexpr int RpcTypeError = 0;//rpc类型错误
	 constexpr int InitMessageFail = 0;//初始化消息失败
	 constexpr int CallFunctionNotExist = 0;//调用方法不存在
	 constexpr int CallArgsError = 0;//调用参数错误
	 constexpr int NotFindUser = 0;//没有找到玩家对象
	 constexpr int CallTypeError = 0;//调用参数类型错误
	 constexpr int CallTimeout = 0;//超时自动调用
	 constexpr int ThrowError = 0;//抛出一个错误
	 constexpr int CallLuaFunctionFail = 0;//调用lua方法错误
	 constexpr int JsonCastProtoFailure = 0;//json转protobuf失败
	 constexpr int ProtoCastJsonFailure = 0;//protobuf转json失败
	 constexpr int CreateProtoFailure = 0;//创建protobuf失败
	 constexpr int SerializationFailure = 0;//序列化失败
	 constexpr int SendMessageFail = 0;//发送消息错误
	 constexpr int NetWorkError = 0;//网络错误
	 constexpr int ParseJsonFailure = 0;//解析json失败
	 constexpr int MysqlResultIsNull = 0;//Mysql结果返回空
	 constexpr int MysqlTableNameError = 0;//Mysql表名不符合规则
	 constexpr int MysqlInitTaskFail = 0;//Mysql初始化失败
	 constexpr int MysqlSocketIsNull = 0;//Mysql连接是空
	 constexpr int MysqlInvokeFailure = 0;//Mysql执行sql语句失败
	 constexpr int SaveToMysqlFailure = 0;//保存数据到Mysql失败
	 constexpr int SaveToRedisFailure = 0;//保存数据到Redis失败
	 constexpr int AccountAlreadyExists = 0;//账号已经存在
	 constexpr int AccountNotExists = 0;//账号不存在
	 constexpr int AddressAllotFailure = 0;//服务分配失败
	 constexpr int LuaCoroutineWait = 0;//等待lua协程完成
};