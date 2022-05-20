git clone https://github.com/protocolbuffers/protobuf.git
git clone https://github.com/lua/lua.git
git clone https://github.com/chriskohlhoff/asio.git
git clone https://github.com/gabime/spdlog.git
cd Libs

$ git clone https://github.com/mongodb/mongo-c-driver.git
$ cd mongo-c-驱动程序
$ git checkout 1.21.1 # 构建特定版本
$ python build/calc_release_version.py > VERSION_CURRENT
$ mkdir cmake-build
$ cd cmake-build
$ cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..