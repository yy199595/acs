./protoc.exe "./db.proto" --cpp_out=../../Generate/Protocol
./protoc.exe ./s2s.proto --cpp_out=../../Generate/Protocol
./protoc.exe ./com.proto --cpp_out=../../Generate/Protocol
./protoc.exe ./c2s.proto --cpp_out=../../Generate/Protocol
pause