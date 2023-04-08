echo "start build server..."
read cmd
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
        cp -R include/openssl ../Libs/openssl/
    fi
done

cd $current_path || exit
# shellcheck disable=SC2038
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete
cmake ./CMakeLists.txt

make lua
make spdlog
make libprotobuf
if ! command -v protoc &> /dev/null
then
    make protoc && install
fi
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
