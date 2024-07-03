rm -rf ./nohup.out

ulimit -c unlimited
export COREDUMP_FILENAME=./

./app --config=./config/run/cent.json --id=1 &
./app --config=./config/run/data.json --id=2 &
./app --config=./config/run/chat.json --id=3 &
./app --config=./config/run/http.json --id=4 &
./app --config=./config/run/gate.json --id=5 &