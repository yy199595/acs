cd ./Libs || exit

git clone -b v3.7.1 https://github.com/protocolbuffers/protobuf.git
cd ./protobuf/cmake || exit
cmake -Dprotobuf_BUILD_TESTS=OFF CMakeLists.txt
make

cd ../../

git clone -b v5.3.6 https://github.com/lua/lua.git

cd lua || exit
make

cd ../

git clone https://github.com/chriskohlhoff/asio.git
git clone -b v1.10.0 https://github.com/gabime/spdlog.git
git clone https://github.com/Tencent/rapidjson.git
git clone -b v2.0.6 https://github.com/microsoft/mimalloc.git
cd ./mimalloc || exit
cmake ./CMakeLists.txt
make
cd ../../
cp ./Libs/protobuf/cmake/libprotobuf.a ./Libs/lib
cp ./Libs/protobuf/cmake/protoc ./bin/proto