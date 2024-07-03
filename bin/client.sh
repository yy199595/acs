port=8080
ip=101.34.67.141
nohup ./app --config=./config/run/client.json --count=50 --host=http://$ip:$port &