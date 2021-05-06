cmake_minimum_required(VERSION 3.1.3)
PROJECT(Server)

ADD_SUBDIRECTORY(Common)
#ADD_SUBDIRECTORY(Server/ProxyServer)
ADD_SUBDIRECTORY(Server/LoginServer)
ADD_SUBDIRECTORY(Server/DataServer)
ADD_SUBDIRECTORY(Server/CenterServer)
ADD_SUBDIRECTORY(Server/GM)