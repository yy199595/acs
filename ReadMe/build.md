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