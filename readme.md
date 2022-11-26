# Sentry 是一个轻量级，分布式rpc服务器框架
# 特点：
### 1. 跨平台
支持windows，Linux，Mac(支持m1和intel芯片)
### 2. 协程
使用汇编实现高效协程，同步方式写异步代码，避免使用回调  
使用共享栈(一个协程大约2000字节)，只要内存足够，理论上开启数量无上限  
基于协程实现了异步rpc和数据库操作
### 3. 服务
支持rpc服务和http服务  
使用c++或者lua实现服务，可以使用lua替换c++服务方法实现  
服务通过配置方式加载，可以运行在任意进程  
开发时候可以所有服务可以运行在一个进程，发布的时候可以部署在多个进程
### 3. http
asio实现异步http客户端，在c++和lua层都可以  
asio实现了一套http1.1协议，实现了GET,POST方法，支持c++和lua处理，支持使用lua替换c++
### 4. 数据库
使用asio实现了redis客户端协议，异步不阻塞  
使用asio实现了mongo客户端协议，实现了增删查改基本操作，异步不阻塞  
封装了mysql相关，通过指定一个protobuf结构创建sql表，新增字段也会自动创建
### 5. 协议
rpc通信基于自己实现的二进制协议  
通过反射可以在lua层热更协议，不需要生成c++代码，protobuf结构体可以直接转lua表
### 6. 脚本
基于原生api实现的c++和lua交互引擎  
支持把c++的方法，类导给lua使用，在c++中等待lua协程完成
# 编译和运行:
### Windows
1. 安装CMake
2. 执行根目录下build.bat
3. 使用vs打开项目,编译lua和protobuf,最后编译app
4. 设置工作目录./bin,启动参数./config/server.json Server
### Mac or Linux
1. 安装CMake,g++
2. 执行根目录下build.sh
3. cd到bin目录 ./app ./config/server.json Server

# [[教程]](https://www.baidu.com)