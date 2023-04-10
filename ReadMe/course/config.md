# <center>配置文件说明<center>
### 框架地址：[https://github.com/YueJianZhenG/Sentry](https://github.com/YueJianZhenG/Sentry)
    服务器启动配置 : bin/config/config.json
        1.分为公共配置和进程专属配置
        2.公共部分包括线程数量,帧率,用户信息,注册中心地址和一些路径信息
        3.进程专属部分包括,服务器组配置文件路径,服务配置文件路径,数据库配置文件路径等
        4.进程专属还包括lua配置,common路径下是一些公用方法,module存放一些自定义服务,component存放自定义方法,main是启动文件路径  

    服务器组配置  : bin/config/cluster/cluster.json
        1.配置每个进程需要加载的服务和组建
        2.Service下字段为true表示创建服务,false表示该进程依赖此服务,启动时会等待该服务启动完成

    服务配置之rpc : bin/config/service/rpc.json
        1.配置所有rpc服务的接口(lua和c++)的信息
        2.Async是否在协程中运行,IsOpen表示接口是否开启,request请求参数名字,response响应参数名字

    服务配置之http : bin/config/service/http.json
        1.配置所有http服务的接口(lua和c++)的信息
        2.Async是否在协程中运行,Type请求的方法,Path请求的路径,Content请求的参数类型