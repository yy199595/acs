cmake_minimum_required(VERSION 3.0)

project(wolfssl)

file(GLOB ssl_source_file "wolfcrypt/src/*.c")

foreach (file ${ssl_source_file})
    message(${file})
endforeach ()

add_library(wolfssl STATIC ${ssl_source_file})