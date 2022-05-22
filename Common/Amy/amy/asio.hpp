#ifndef __AMY_ASIO_HPP__
#define __AMY_ASIO_HPP__

#if !defined(USE_BOOST_ASIO) || (USE_BOOST_ASIO == 0)

#include "asio/basic_io_object.hpp"
#include "asio/io_service.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/local/stream_protocol.hpp"
#include "asio/placeholders.hpp"

#include <system_error>

#else

#include <asio/basic_io_object.hpp>
#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/local/stream_protocol.hpp>
#include <asio/placeholders.hpp>
#include <system/system_error.hpp>

#endif

#endif // __AMY_ASIO_HPP__

// vim:ft=cpp sw=4 ts=4 tw=80 et
