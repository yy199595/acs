### Windows
1. 安装CMake
2. 执行根目录下build.bat,进入build目录
3. 使用vs打开项目,编译lua和protobuf,最后编译app
4. 设置工作目录./bin,点击本地windows调试器启动
5. 单进程启动设置app为启动项目运行，多进程启动./start.bat
### Linux
1. 安装CMake,g++
2. 执行根目录下build.sh,进入到bin目录
3. build.sh openssl编译openssl并使用，依赖openssl
4. 单进程启动执行运行app，多线程启动执行./start.bat 
5. tail -f ./nohup.out查看运行日志
### Mac
1. 安装CMake,g++,clion
2. 打开clion,选择打开项目所在CMakeLists.txt文件
3. 编译项目app,点击运行(有默认启动参数)

### 启动参数讲解
1.--db数据库的ip地址(redis,mongodb等都是这个地址)(默认:127.0.0.1)
2.--rpc内网rpc监听地址,监听内网连接
3.--http外网监听的http地址,处理外网http连接
4.--gate外网监听的网关地址(支持自定义tcp和websocket)
5.--ws内网监听的地址,使用ws协议
6.--kcp内网监听的kcp地址,使用kcp协议
7.--udp内网监听的udp地址,使用udp协议
8.--lua脚本的入口文件
9.--cluster集群的配置文件,集群配置包含哪个进程启动哪些服务(默认:debug)
10.--id当前服务器的id(默认:0)
11.--name当前进程的名字(默认:server)