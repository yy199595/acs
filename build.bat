del /F /S /Q CMakeCache.txt
IF NOT EXIST build (
  MKDIR build
)
cd ./build

cmake ../CMakeLists.txt
pause