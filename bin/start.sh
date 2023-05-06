rm -rf ./log
rm -rf ./nohup.out
nohup ./app ./config/run/registry.json &
nohup ./app ./config/run/data.json &
nohup ./app ./config/run/http.json &
nohup ./app ./config/run/chat.json &
nohup ./app ./config/run/gate.json &
tail -f ./nohup.out