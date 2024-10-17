#!/bin/bash

# shellcheck disable=SC2124
cmd="$@"
start_time=$(date)
current_path=$(pwd)
cd $current_path || exit

if [ ! -n "$1" ] ;then
  echo "***********************************************************"
  echo "*                                                         *"
  echo "*           debug        build server type=debug          *"
  echo "*           release      build server type=release        *"
  echo "*           lib          build protobuf and jemalloc      *"
  echo "*           mysql        download mysql and build         *"
  echo "*           jemalloc     download jemalloc and build      *"
  echo "*           openssl      download openssl and build       *"
  echo "*           wolfssl      download wolfssl and build       *"
  echo "*                                                         *"
  echo "***********************************************************"
else
    cmake ./CMakeLists.txt
    echo "start build server..."
fi
for arg in $cmd; do
    if [[ $arg == "jemalloc" ]]; then
        cd ./Libs/ || exit
        if [ -d "./Libs/jemalloc" ]; then
            echo "jemalloc already exists local"
        else
            git clone https://github.com/jemalloc/jemalloc.git
        fi
        cd ./jemalloc || exit
        ./autogen.sh
        ./configure
        make
    fi

    if [[ $arg == "mysql" ]]; then
        cd "$current_path" || exit
        if [ -d "./Libs/mysql/lib" ]; then
            mkdir -p "./Libs/mysql/lib"
        fi
        if [ -d "./Libs/bin/mysql-connector-c-6.1.11-src" ]; then
            echo "mysql already exists locally"
        else
            cd ./Libs/bin/ || exit
            wget http://cdn.tarsyun.com/src/mysql-connector-c-6.1.11-src.fixed.zip
            unzip mysql-connector-c-6.1.11-src.fixed.zip
            rm -rf mysql-connector-c-6.1.11-src.fixed.zip
        fi
        # shellcheck disable=SC2086
        cd $current_path/Libs/bin/mysql-connector-c-6.1.11-src || exit
        cmake ./CMakeLists.txt
        make
        cd "$current_path" || exit
    fi

    if [[ $arg == "openssl" ]]; then
        cd ./Libs/bin || exit
        if [ -d "./Libs/bin/openssl" ]; then
            echo "openssl already exists locally"
        else
            git clone https://github.com/openssl/openssl.git
        fi
        cd ./openssl || exit
        ./config
        make
         if [ -d "../../openssl/lib" ]; then
            mkdir -p "../../openssl/lib"
        fi
        cp -r ./*.a ../../openssl/lib
        cd "$current_path" || exit
        rm -rf ./Lib/bin/openssl
    fi

    if [[ $arg == "lib" ]]; then
        cd "$current_path" || exit
        make protoc
        make libprotobuf
    fi

    if [[ $arg == "wolfssl" ]]; then
        cd "$current_path" || exit
        make wolfssl
    fi

    if [[ $arg == "debug" ]]; then
        cd "$current_path" || exit
        cmake -DCMAKE_BUILD_TYPE=Debug ./CMakeLists.txt
        make app
    fi

    if [[ $arg == "release" ]]; then
          cd "$current_path" || exit
          cmake -DCMAKE_BUILD_TYPE=Release ./CMakeLists.txt
          make app
    fi
done

# shellcheck disable=SC2038
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete

end_time=$(date)
# shellcheck disable=SC2046
elapsed_time=$(expr $(date -d "$end_time" +%s) - $(date -d "$start_time" +%s))

# Print the elapsed time in seconds
echo "Elapsed time: $elapsed_time seconds"
