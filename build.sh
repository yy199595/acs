echo "start build server..."
# shellcheck disable=SC2124
cmd="$@"
start_time=$(date)
current_path=$(pwd)
cd $current_path || exit
# Define a string array

for arg in $cmd; do
    cd $current_path || exit
    if [[ $arg == "jemalloc" ]]; then
        cd ./Libs/ || exit
        if [ -d "./Libs/jemalloc" ]; then
            echo "jemalloc already exists locally"
        else
            git clone https://github.com/jemalloc/jemalloc.git
        fi
        cd ./jemalloc || exit
        ./autogen.sh
        ./configure
        make
    fi

    if [[ $arg == "mysql" ]]; then
        wget http://cdn.tarsyun.com/src/mysql-connector-c-6.1.11-src.fixed.zip
    fi

    if [[ $arg == "openssl" ]]; then
        cd ./Libs/ || exit
        if [ -d "./Libs/openssl" ]; then
            echo "openssl already exists locally"
        else
            git clone https://github.com/openssl/openssl.git
        fi
        cd ./openssl || exit
        ./config
        make
    fi

    if [[ $arg == "lib" ]]; then
        make lua
        make spdlog
        make libprotobuf
    fi
done

cd $current_path || exit
# shellcheck disable=SC2038
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete
cmake ./CMakeLists.txt

make app

# shellcheck disable=SC2038
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete

end_time=$(date)
# shellcheck disable=SC2046
elapsed_time=$(expr $(date -d "$end_time" +%s) - $(date -d "$start_time" +%s))

# Print the elapsed time in seconds
echo "Elapsed time: $elapsed_time seconds"
