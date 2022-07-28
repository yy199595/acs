  
echo "start build server..."
# shellcheck disable=SC2038
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete
cmake ./CMakeLists.txt

make lua
make libprotobuf
make Sentry

# shellcheck disable=SC2038
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete

echo "build server successful..."
