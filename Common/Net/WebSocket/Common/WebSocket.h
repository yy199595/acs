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

	constexpr int PING_TIME = 30; //秒
	constexpr int MESSAGE_MAX_COUNT = 1024 * 1024; //消息最大字节

	namespace header
	{
		constexpr const char * Upgrade = "Upgrade";
		constexpr const char * WebSocket = "websocket";
		constexpr const char * Connection = "Connection";
		constexpr const char * SecWebSocketKey = "Sec-WebSocket-Key";
		constexpr const char * SecWebSocketAccept = "Sec-WebSocket-Accept";
		constexpr const char * SecWebSocketVersion = "Sec-WebSocket-Version";
	}
}


#endif //APP_WEBSOCKET_H
