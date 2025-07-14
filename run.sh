#!/bin/bash

# shellcheck disable=SC2124
cmd="$@"
start_time=$(date)
current_path=$(pwd)
cd $current_path || exit

for arg in $cmd; do
     if [[ $arg == "ps" ]]; then
          ps aux|grep app | awk '{print $6/1024 "MB"}'
     fi

     if [[ $arg == "htop" ]]; then
          htop -p $(pidof app)
     fi
     if [[ $arg == "vld" ]]; then
        cd bin
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=./a.txt ./app --http=8080 --gate=7789 --console=1
     fi
done
