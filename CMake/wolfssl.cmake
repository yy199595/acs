
option(WOLFSSL_EXAMPLES OFF)
add_definitions(-D ASIO_USE_WOLFSSL)
add_definitions(-D WOLFSSL_HAVE_MIN)
add_definitions(-D WOLFSSL_HAVE_MAX)
#add_definitions(-D HAVE_WEBSERVER)
#add_definitions(-D HAVE_MEMCACHED)
#add_definitions(-D HAVE_LIGHTY)
add_definitions(-D OPENSSL_EXTRA)
add_definitions(-D WOLFSSL_TYPES_DEFINED)

add_definitions(-D OPENSSL_ALL)
add_definitions(-D WOLFSSL_ASIO)
#add_definitions(-D OPENSSL_EXTRA)
add_definitions(-D WOLFSSL_STATIC)


include_directories(Libs/wolfssl)
include_directories(Libs/bin/wolfssl)
include_directories(Libs/wolfssl/wolfssl)

add_definitions(-D __ENABLE_WOLF_SSL__)

add_subdirectory(${PROJECT_SOURCE_DIR}/Libs/wolfssl ${PROJECT_SOURCE_DIR}/Libs/bin/wolfssl)