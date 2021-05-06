username=`whoami`
pidset=`ps -ef | grep $username |grep -v grep | grep -E '(yjz_.*)' | awk '{ print $2 }'`
pidset=`echo $pidset `

echo "pids: $pidset"

echo "start kill pid..."

for pid in $pidset
do
	cids=`ps -ef | grep $pid | grep -v grep | awk '{ print $2 }'`
	cids=`echo $cids `
	for cid in $cids
	do
		kill -USR1 $cid
		sleep 1
	done
done
screen -wipe
echo "done"