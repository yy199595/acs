rm -rf ./nohup.out

ulimit -c unlimited
export COREDUMP_FILENAME=./
sysctl -w kernel.core_pattern="core.%t"

nginx -s stop
pkill -15 nginx
nginx -c /home/yjz/tendo/bin/config/nginx.conf

nohup ./app --config=./config/run/all.json --rpc=7788 --http=8088 --db=127.0.0.1 &
#nohup ./app --config=./config/run/data.json --id=2 &
#nohup ./app --config=./config/run/chat.json --id=3 &
#nohup ./app --config=./config/run/http.json --id=4 &
#nohup ./app --config=./config/run/gate.json --id=5 &
#nohup ./app --config=./config/run/web.json --id=6 &