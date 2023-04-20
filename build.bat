del /F /S /Q CMakeCache.txt
IF NOT EXIST build (
  MKDIR build
)
cd ./build

cmake -DCMAKE_BUILD_TYPE=Debug ../CMakeLists.txt
pause