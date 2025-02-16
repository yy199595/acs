//
// Created by 64658 on 2025/2/11.
//
#include <algorithm>
#include "MysqlCommon.h"
namespace mysql
{
	bool HandshakeResponse::OnDecode(const std::string& packet)
	{
		size_t offset = 0;

		// 解析协议版本
		this->protocol_version = packet[offset];

		// 解析服务器版本（null 结尾字符串）
		size_t server_version_end = std::find(packet.begin() + 1, packet.end(), '\0') - packet.begin();
		this->server_version = std::string(packet.begin() + 1, packet.begin() + server_version_end);
		offset = server_version_end + 1;

		// 解析连接 ID
		this->connection_id = *reinterpret_cast<const uint32_t*>(&packet[offset]);
		offset += 4;

		// 解析盐值第一部分（8 字节）
		std::string salt_part1(packet.begin() + offset, packet.begin() + offset + 8);
		offset += 8;

		// 跳过填充值（1 字节）
		offset += 1;

		// 解析服务器能力标志（低 16 位）
		uint16_t capability_flags_low = *reinterpret_cast<const uint16_t*>(&packet[offset]);
		offset += 2;

		// 解析字符集
		this->character_set = packet[offset];
		offset += 1;

		// 解析服务器状态
		this->status_flags = *reinterpret_cast<const uint16_t*>(&packet[offset]);
		offset += 2;

		// 解析服务器能力标志（高 16 位）
		uint16_t capability_flags_high = *reinterpret_cast<const uint16_t*>(&packet[offset]);
		offset += 2;
		this->capability_flags = (capability_flags_high << 16) | capability_flags_low;

		// 解析认证插件数据长度
		uint8_t auth_plugin_data_len = packet[offset];
		offset += 1;

		// 跳过保留字段（10 字节）
		offset += 10;

		// 解析盐值第二部分（12 字节）
		std::string salt_part2(packet.begin() + offset, packet.begin() + offset + 12);
		offset += 12;

		// 组合完整盐值
		this->salt = salt_part1 + salt_part2;

		// 解析认证插件名称（null 结尾字符串）
		this->auth_plugin_name = std::string(packet.c_str() + offset + 1, auth_plugin_data_len);
		return true;
	}
}