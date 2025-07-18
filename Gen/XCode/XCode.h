#pragma once
namespace XCode 
{
	 constexpr int Ok = 0;//成功
	 constexpr int Failure = 1;//失败
	 constexpr int ConfigError = 2;//配置错误
	 constexpr int NetTimeout = 3;//网络超时
	 constexpr int NetReadFailure = 4;//接收网络消息失败
	 constexpr int NetSendFailure = 5;//发送网络消息失败
	 constexpr int NetConnectFailure = 6;//连接服务器失败
	 constexpr int NetActiveShutdown = 7;//主动关闭连接
	 constexpr int NetBigDataShutdown = 8;//数据包过大
	 constexpr int ParseMessageError = 9;//解析protobuf失败
	 constexpr int UnKnowPacket = 10;//恶意数据包
	 constexpr int ParseRequestDataError = 11;//解析请求数据包失败
	 constexpr int ParseResponseDataError = 12;//解析响应数据包失败
	 constexpr int DataBaseCreateFailure = 13;//创建数据库失败
	 constexpr int DataBaseInsertFailure = 14;//插入数据到数据库失败
	 constexpr int DataBaseUpdateFailure = 15;//更新数据到数据库失败
	 constexpr int DataBaseSaveFailure = 16;//保存数据到数据库失败
	 constexpr int DataBaseDeleteFailure = 17;//删除数据到数据库失败
	 constexpr int DataBaseFindDataEmpty = 18;//从数据库查取不到数据
	 constexpr int CallServiceNotFound = 19;//调用服务未发现
	 constexpr int NotFoundRpcConfig = 20;//没有rpc配置
	 constexpr int RpcTypeError = 21;//rpc类型错误
	 constexpr int InitMessageFail = 22;//初始化消息失败
	 constexpr int CallFunctionNotExist = 23;//调用方法不存在
	 constexpr int CallArgsError = 24;//调用参数错误
	 constexpr int NotFindUser = 25;//没有找到玩家对象
	 constexpr int CallTypeError = 26;//调用参数类型错误
	 constexpr int CallTimeout = 27;//超时自动调用
	 constexpr int ThrowError = 28;//抛出一个错误
	 constexpr int CallLuaFunctionFail = 29;//调用lua方法错误
	 constexpr int JsonCastProtoFailure = 30;//json转protobuf失败
	 constexpr int ProtoCastJsonFailure = 31;//protobuf转json失败
	 constexpr int CreateProtoFailure = 32;//创建protobuf失败
	 constexpr int SerializationFailure = 33;//序列化失败
	 constexpr int SendMessageFail = 34;//发送消息错误
	 constexpr int NetWorkError = 35;//网络错误
	 constexpr int ParseJsonFailure = 36;//解析json失败
	 constexpr int AccountAlreadyExists = 37;//账号已经存在
	 constexpr int AccountNotExists = 38;//账号不存在
	 constexpr int AddressAllotFailure = 39;//服务分配失败
	 constexpr int LuaCoroutineWait = 40;//等待lua协程完成
	 constexpr int MakeTcpRequestFailure = 41;//创建tcp请求错误
	 constexpr int MakeHttpRequestFailure = 42;//创建http请求错误
	 constexpr int ParseHttpUrlFailure = 43;//解析http的url失败
	 constexpr int UnknownMessageNetType = 44;//未知的传输网络类型
	 constexpr int OnlyUseTcpProtocol = 45;//只能使用tcp协议
	 constexpr int NotFoundServerRpcAddress = 46;//找不到服务器rpc地址
	 constexpr int NotFoundPlayerRpcAddress = 47;//找不到玩家所在服务器地址
	 constexpr int NotFoundActor = 48;//找不到Actor
	 constexpr int NotFoundActorAddress = 49;//找不到actor地址
	 constexpr int AccountPasswordError = 50;//账号不存在或者密码错误
	 constexpr int PlayerOnLine = 51;//玩家已经在线
	 constexpr int PlayerNotOnLine = 52;//玩家不在线
	 constexpr int CallFrequently = 53;//调用太频繁
	 constexpr int NotFoundSender = 54;//找不到Sender对象
	 constexpr int ConnectionComeOnline = 55;//连接数达到上线
	 constexpr int DeleteData = 56;//释放内存
	 constexpr int CloseSocket = 57;//关闭网络连接
	 constexpr int ConnectTimeout = 58;//连接超时
	 constexpr int ReadDataTimeout = 59;//读取数据超时
	 constexpr int SendDataTimeout = 60;//发送数据超时
	 constexpr int CodeNotExist = 61;//验证码不存在
	 constexpr int TokenInvalid = 62;//Token无效
	 constexpr int TokenExpTime = 63;//Token已经过期
	 constexpr int NotFoundData = 64;//找不到数据
	 constexpr int CreateOrderFrequently = 65;//创建订单太过于频繁
	 constexpr int OrderAlreadyRefund = 66;//订单已经退款
	 constexpr int RequestWxApiError = 67;//请求微信api错误
	 constexpr int PermissionDenied = 68;//权限不足
	 constexpr int AuthPluginNonsupport = 69;//验证方式不支持
	 constexpr int PasswordAuthFail = 70;//密码验证失败
};