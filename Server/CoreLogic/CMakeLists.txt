cmake_minimum_required(VERSION 3.1.3)
PROJECT(CoreLogic)

aux_source_directory(./Manager SUPER_SRC)

include_directories(./)
include_directories(./Manager SUPER_SRC)
