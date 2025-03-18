# <center>配置文件说明<center>
### 框架地址：[https://github.com/yy199595/acs](https://github.com/yy199595/acs)
    服务器启动配置 : bin/config/run/xxx.json
        1.分为公共配置(com.json,log.json等)和进程专属配置
        3.启动多进程,需要增加每一个配置,指定lua入门文件和进程名字还有集群配置

    服务器组配置  : bin/config/cluster/xxx.json
        1.配置每个进程需要加载的服务和组建
        2.component表示进程需要加载的组件,rpc表示进程加载的rpc服务器,http表示进程要加载的http服务

    服务配置之rpc : bin/config/service/rpc.json
        1.配置所有rpc服务的接口(lua和c++)的信息
        2.async是否在协程中运行,bind表示协议交给哪个回调处理,open表示接口是否开启,request(使用pb需要指定)请求参数名字,response(使用pb需要指定)响应参数名字
        3.proto表示使用的协议,目前支持(pb,json,string,lua)等,可扩展
        4.client表示是否可以被客户端访问,to_client表示这个消息是不是发送到服务器的
    服务配置之http : bin/config/service/http.json
        1.配置所有http服务的接口(lua和c++)的信息
        2.async是否在协程中运行,bind表示协议交给哪个回调处理,type请求的方法,content-type请求的参数类型,auth表示支持请求是否需要验证