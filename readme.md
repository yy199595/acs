## Sentry 是一个轻量级，分布式rpc服务器框架
## 特点
    1.跨平台(win,mac,linux),无依赖
    2.基于协程(汇编实现,支持千万级别并发)
    3.强大的异步支持c++和lua(rpc,数据库操作)
    4.基于组件思想(可以根据配置加载不同组件组成新的进程)
    5.http1.1协议(实现GET,POST方法,支持lua和c++异步处理)
    6.内置mongoclient(使用asio实现),redisclient(使用asio实现),无依赖
    7.基于protobuf通信(利用反射可以在lua层热更协议,不需要生成C++代码)
    8.内置lua脚本系统，可以使用lua替换c++服务，也可以全部使用lua实现服务
##     编译和运行:
## Windows
    1.安装CMake
    2.执行根目录下build.bat
    3.使用vs打开项目,编译lua和protobuf,最后编译app
    4.设置工作目录./bin,启动参数./config/server.json Server
## Mac or Linux
    1.安装CMake,g++
    2.执行根目录下build.sh
    3.cd到bin目录 ./app ./config/server.json Server