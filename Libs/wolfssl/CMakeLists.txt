cmake_minimum_required(VERSION 3.0)

project(wolfssl)

file(GLOB ssl_header_file "wolfssl/*.h")
file(GLOB ssl_header_file "wolfssl/*/*.h")
file(GLOB ssl_source_file "wolfcrypt/src/*.c")

foreach (file ${ssl_header_file})
    message(${file})
endforeach ()

add_library(wolfssl STATIC ${ssl_header_file} ${ssl_source_file})