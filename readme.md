## Sentry 是一个轻量级，分布式rpc服务器框架
## 特点
    1.基于汇编实现的一套协程调度框架，支持千万级并发,
    2.基于组件思想可以加载不同组件和服务，组成不同节点，服务间使用rpc通信
    ```#cpp#
    this-
        
    3.内置mongoclient(使用asio实现),redisclient(使用asio实现)，不需要依赖三方
    4.内置一套简单的http1.0协议，实现GET,POST方法
    5.服务之需要通过配置就可以运行在任意节点，实现了服务注册与发现
    6.跨平台支持(linux,mac(包括m1和inter),win)
    7.内置lua脚本系统，可以使用lua拦截c++服务，使用c++实现的方法出问题可以使用lua重写
## 编译和运行:
    1.需要c++14标准,CMake
    2.win下执行build.bat然后使用vs打开,编译protobuf这个项目,编译lua这个项目,最后编译app完成
    3.linux和mac下直接执行build.sh
    4.win下运行设置工作目录到./bin, 启动参数./config/server.json Server
    5.mac和linux cd到bin目录，执行./app ./config/server.json Server
##  实现一个rpc服务
    打开bin/config/service/rpc.json 把这个服务和配置进去
    {
	"XXXService": {
		"xxx_func": {
			"Async": false, //是否需要启动一个协程去执行
			"Type": "Server", //server 只能被服务器调用
			"Request": "com.type.int64", //请求参数(可以为空)
			"Response": "s2s.allot.response" //返回参数(可以为空)
		}
	},
## C++服务
    ```cpp
    class XXXService final : public LocalRpcService
    {
    public:
		XXXService() = default;
    public:
        XCode xxx_func(const com::type::int64 & userId, s2s::allot::response & response); //被rpc服务调用的方法
	private:
		bool OnStart() final; //在服务启动的时候调用，用于注册
        bool OnClose() final;   //在服务关闭的时候调用，用于清理
	 private:
    }

    bool XXXService::OnStart()
	{
        BIND_COMMON_RPC_METHOD(XXXService::xxx_func); //注册c++服务
        return true;
	} 
    ```

##  lua服务
    打开bin/config/service/rpc.json 把这个服务和配置进去
    {
	"XXXService": {
		"xxx_func": {
			"Async": false, //是否需要启动一个协程去执行
			"Type": "Server", //server 只能被服务器调用
			"Request": "com.type.int64", //请求参数(可以为空)
			"Response": "s2s.allot.response" //返回参数(可以为空)
		}
	},
    在bin/script/Server/Service/rpc 下新建XXXService.lua
    XXXService = {}
    XXXService.xxx_func = function(request)
        return XCode.Successful, { }
    end

##  调用实现的的rpc服务 把服务加到bin/cluster/cluster.json中某个节点
    ```cpp
    com::type::int64 request;
    std::shared<s2s::allot::response> response(new s2s::allot::response())
    XXXService * rpcService = this->mApp->GetService("XXXService")
    XCode code = rpcService->Call("127.0.0.1:7788", request, response)
    // 第一个参数是服务所在进程地址,第二个是请求参数，第三个是响应参数, 调用过程中会自动挂起协程，不会阻塞主线程
     ```
