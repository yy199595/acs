screen -dms yjz.registry ./app ./config/server.json RegistryServer
screen -dms yjz.data ./app ./config/server.json DataServer
screen -dms yjz.http ./app ./config/server.json HttpServer
screen -dms yjz.chat ./app ./config/server.json ChatServer
screen -dms yjz.gate ./app ./config/server.json GateServer
screen -dms yjz.log ./app ./config/server.json LogServer