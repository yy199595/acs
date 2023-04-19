rm -rf ./log
rm -rf ./nohup.out
nohup ./app ./config/run/cluster.json RegistryServer &
nohup ./app ./config/run/cluster.json DataServer &
nohup ./app ./config/run/cluster.json HttpServer &
nohup ./app ./config/run/cluster.json ChatServer &
nohup ./app ./config/run/cluster.json GateServer &
nohup ./app ./config/run/cluster.json LogServer &
tail -f ./nohup.out