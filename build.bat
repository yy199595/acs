del /F /S /Q CMakeCache.txt
IF NOT EXIST build (
  MKDIR build
)
cd ./build


cmake -DCMAKE_BUILD_TYPE=Debug ../CMakeLists.txt

::set path==%path%;C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE
::devenv.com .\app.sln /Build
pause