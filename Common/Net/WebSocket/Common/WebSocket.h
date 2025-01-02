//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_WEBSOCKET_H
#define APP_WEBSOCKET_H

namespace ws
{
	constexpr unsigned char OPCODE_TEXT = 0x1;
	constexpr unsigned char OPCODE_BIN = 0x2;
	constexpr unsigned char OPCODE_CLOSE = 0x8;
	constexpr unsigned char OPCODE_PING = 0x9;
	constexpr unsigned char OPCODE_PONG = 0xA;
}

#endif //APP_WEBSOCKET_H
