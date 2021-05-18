cmake_minimum_required(VERSION 3.1.3)
PROJECT(ServerData)

aux_source_directory(./XCode SUPER_SRC)
aux_source_directory(./Protocol SUPER_SRC)

include_directories(./)
include_directories(./XCode)
include_directories(./Protocol)
