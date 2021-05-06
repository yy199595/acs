echo "start all .... "
screen -dmS "yjz_proxysrv" ./ProxySrv
screen -dmS "yjz_datasrv" ./DataSrv
screen -dmS "yjz_loginsrv" ./LoginSrv

screen -ls yjz
echo "done"