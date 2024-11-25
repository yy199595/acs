
ulimit -c unlimited
export COREDUMP_FILENAME=./
sysctl -w kernel.core_pattern="core.%t"

./app --rpc=7788 --http=8088 --gate=7789 --lua=Client