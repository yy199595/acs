#!/bin/bash

# 创建存储数据的目录
mkdir -p ./data/mysql_data
mkdir -p ./data/redis_data
mkdir -p ./data/mongo_data

# 定义容器的名称
mysql_container="mysql-container"
redis_container="redis-container"
mongo_container="mongo-container"

# 停止并删除 MySQL 容器
if [ "$(docker ps -q -f name=$mysql_container)" ]; then
    echo "Stopping and removing existing MySQL container..."
    docker stop $mysql_container
    docker rm $mysql_container
fi

# 重新启动 MySQL 容器，并挂载数据存储路径
echo "Starting MySQL container..."
docker run -d --name $mysql_container -p 3306:3306 \
    -e MYSQL_ROOT_PASSWORD=199595yjz \
    -e MYSQL_DATABASE=mydatabase \
    -e MYSQL_USER=yjz \
    -e MYSQL_PASSWORD=199595yjz \
    -v $(pwd)/data/mysql_data:/var/lib/mysql \
    mysql:5.7

# 停止并删除 Redis 容器
if [ "$(docker ps -q -f name=$redis_container)" ]; then
    echo "Stopping and removing existing Redis container..."
    docker stop $redis_container
    docker rm $redis_container
fi

# 重新启动 Redis 容器，并挂载数据存储路径
echo "Starting Redis container..."
docker run -d --name $redis_container -p 6379:6379 \
    -e REDIS_PASSWORD=199595yjz \
    -v $(pwd)/data/redis_data:/data \
    redis

# 停止并删除 MongoDB 容器
if [ "$(docker ps -q -f name=$mongo_container)" ]; then
    echo "Stopping and removing existing MongoDB container..."
    docker stop $mongo_container
    docker rm $mongo_container
fi

# 重新启动 MongoDB 容器，并挂载数据存储路径
echo "Starting MongoDB container..."
docker run -d --name $mongo_container -p 27017:27017 \
    -e MONGO_INITDB_ROOT_USERNAME=yjz \
    -e MONGO_INITDB_ROOT_PASSWORD=199595yjz \
    -v $(pwd)/data/mongo_data:/data/db \
    mongo
