call ./stop.bat
start ./Debug/app.exe ./config/config.json RegistryServer
start ./Debug/app.exe ./config/config.json DataServer
start ./Debug/app.exe ./config/config.json GateServer
start ./Debug/app.exe ./config/config.json ChatServer
start ./Debug/app.exe ./config/config.json HttpServer