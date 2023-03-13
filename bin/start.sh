rm -rf ./out.log
nohup ./app ./config/server.json RegistryServer &
nohup ./app ./config/server.json DataServer &
nohup ./app ./config/server.json HttpServer &
nohup ./app ./config/server.json ChatServer &
nohup ./app ./config/server.json GateServer &
nohup ./app ./config/server.json LogServer &
tail -f ./out.log