rm -rf ./log
rm -rf ./nohup.out
nohup ./app ./config/config.json RegistryServer &
nohup ./app ./config/config.json DataServer &
nohup ./app ./config/config.json HttpServer &
nohup ./app ./config/config.json ChatServer &
nohup ./app ./config/config.json GateServer &
nohup ./app ./config/config.json LogServer &
ps -ux|grep app