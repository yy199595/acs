echo "start build server..."
find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete
cmake ./CMakeLists.txt

make -j2

find ./ -name CMakeFiles | xargs rm -r
find ./ -name cmake_install.cmake -delete
find ./ -name CMakeCache.txt -delete

echo "build server successful..."
