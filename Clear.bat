del /F /S /Q *.obj
del /F /S /Q *.ilk
del /F /S /Q *.pdb
del /F /S /Q *.idb
del /F /S /Q *.exp
del /F /S /Q *.dep
del /F /S /Q *.manifest
del /F /S /Q *.log
del /F /S /Q *.ncb
del /F /S /Q *.suo
del /F /S /Q *.pch
del /F /S /Q *.res
del /F /S /Q server\output_lib\*.lib
del /F /S /Q output.txt
del /F /S /Q cmake_install.cmake
del /F /S /Q CMakeCache.txt
del /F /S /Q BuildLog.htm
del /F /S /Q ReadMe.txt
del /F /S /Q *.old
del /F /S /Q *.dmp
del /F /S /Q *.tlog
del /F /S /Q *.sdf
del /F /S /Q *.vcxproj
del /F /S /Q *.filters
del /F /S /Q *.user
del /F /S /Q *.sln
rmdir  /s/q build
rmdir /s/q .vs
rmdir /s/q CMakeFiles
rmdir /s/q cmake-build-debug
rmdir /s/q Debug
rmdir /s/q Win32
rmdir /s/q X64
rmdir /s/q CMakeFiles
rmdir /s/q app.dir
rmdir /s/q .vs
rmdir /s/q .idea
cd bin
rmdir /s/q Debug
rmdir /s/q log
rm
