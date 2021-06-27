protoc.exe "./db.proto" --cpp_out=../Server/ServerData/Protocol
protoc.exe "./c2s.proto" --cpp_out=../Server/ServerData/Protocol
protoc.exe "./s2s.proto" --cpp_out=../Server/ServerData/Protocol
protoc.exe "./com.proto" --cpp_out=../Server/ServerData/Protocol
pause
