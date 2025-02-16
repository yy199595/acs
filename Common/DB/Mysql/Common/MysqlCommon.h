//
// Created by 64658 on 2025/2/11.
//

#ifndef APP_MYSQLCOMMON_H
#define APP_MYSQLCOMMON_H
#include <string>
#include <utility>
#include <vector>
namespace mysql
{
	namespace cmd
	{
		constexpr char SLEEP = 0x00;             // 使 MySQL 服务器进入休眠状态
		constexpr char QUIT = 0x01;              // 关闭 MySQL 连接
		constexpr char INIT_DB = 0x02;           // 选择数据库 (USE database)
		constexpr char QUERY = 0x03;             // 执行 SQL 语句
		constexpr char FIELD_LIST = 0x04;        // 获取表的字段列表
		constexpr char CREATE_DB = 0x05;         // 创建数据库
		constexpr char DROP_DB = 0x06;           // 删除数据库
		constexpr char REFRESH = 0x07;           // 刷新数据库或表缓存
		constexpr char SHUTDOWN = 0x08;          // 关闭 MySQL 服务器
		constexpr char STATISTICS = 0x09;        // 获取 MySQL 服务器统计信息
		constexpr char PROCESS_INFO = 0x0A;      // 获取正在执行的进程列表
		constexpr char CONNECT = 0x0B;           // 连接 MySQL 服务器
		constexpr char PROCESS_KILL = 0x0C;      // 终止查询进程
		constexpr char DEBUG = 0x0D;             // 调试 MySQL 服务器
		constexpr char PING = 0x0E;              // 检测 MySQL 服务器是否存活
		constexpr char TIME = 0x0F;              // 获取服务器当前时间
		constexpr char DELAYED_INSERT = 0x10;    // 已弃用
		constexpr char CHANGE_USER = 0x11;       // 切换用户
		constexpr char BINLOG_DUMP = 0x12;       // 复制 Binlog
		constexpr char TABLE_DUMP = 0x13;        // 复制表数据（已废弃）
		constexpr char CONNECT_OUT = 0x14;       // 外部连接
		constexpr char REGISTER_SLAVE = 0x15;    // 作为从服务器注册
		constexpr char STMT_PREPARE = 0x16;      // 预处理 SQL 语句（用于 `PREPARE` 语句）
		constexpr char STMT_EXECUTE = 0x17;      // 执行预处理的 SQL 语句
		constexpr char STMT_SEND_LONG_DATA = 0x18; // 发送长数据（如 BLOB）
		constexpr char STMT_CLOSE = 0x19;        // 关闭预处理语句
		constexpr char STMT_RESET = 0x1A;        // 重置预处理语句
		constexpr char SET_OPTION = 0x1B;        // 设置会话选项
		constexpr char STMT_FETCH = 0x1C;        // 获取预处理查询结果
		constexpr char DAEMON = 0x1D;            // MySQL 服务器后台任务
		constexpr char BINLOG_DUMP_GTID = 0x1E;  // Binlog GTID 复制
		constexpr char RESET_CONNECTION = 0x1F;  // 重置连接
	}

	constexpr uint32_t CLIENT_FLAG             = 17037263; //1 0000 0011 1111 0111 1100 1111
}

namespace mysql
{
	namespace charset
	{
		constexpr unsigned char latin1 = 8;  // Latin1字符集，常用于西欧语言，编号为8
		constexpr unsigned char utf8 = 33; // UTF-8字符集，广泛支持多种语言字符，编号为33
		constexpr unsigned char utf8mb4 = 255; // UTF-8mb4字符集，支持完整的Unicode字符集，包括表情符号等，编号为255
		constexpr unsigned char ascii = 1; // ASCII字符集，包含128个字符，主要用于英文字符，编号为1
		constexpr unsigned char gbk = 16; // GBK字符集，常用于简体中文，编号为16
		constexpr unsigned char big5 = 18; // Big5字符集，常用于繁体中文，编号为18
		constexpr unsigned char latin2 = 4; // Latin2字符集，支持中欧语言，编号为4
		constexpr unsigned char sjis = 32; // Shift-JIS字符集，用于日本语言，编号为32
		constexpr unsigned char euckr = 22; // EUC-KR字符集，用于韩国语言，编号为22
		constexpr unsigned char utf32 = 101; // UTF-32字符集，支持所有Unicode字符，编号为101
	}
}

namespace mysql
{
	namespace config
	{
		constexpr unsigned char charset = charset::utf8mb4; //默认字符集
		constexpr uint32_t PACKAGE_MAX_SIZE = 1024 * 1024 * 2; // 默认2M
	}
}

namespace mysql
{
	namespace field
	{
		// 整数类型
		constexpr unsigned char MYSQL_TYPE_TINY           = 0x01;    // TINYINT
		constexpr unsigned char MYSQL_TYPE_SHORT          = 0x02;    // SMALLINT
		constexpr unsigned char MYSQL_TYPE_LONG           = 0x03;    // INT
		constexpr unsigned char MYSQL_TYPE_FLOAT          = 0x04;    // FLOAT
		constexpr unsigned char MYSQL_TYPE_DOUBLE         = 0x05;    // DOUBLE
		constexpr unsigned char MYSQL_TYPE_NULL           = 0x06;    // NULL类型
		constexpr unsigned char MYSQL_TYPE_LONGLONG       = 0x08;    // BIGINT
		constexpr unsigned char MYSQL_TYPE_INT24          = 0x09;    // MEDIUMINT
		constexpr unsigned char MYSQL_TYPE_YEAR           = 0x0D;    // YEAR
		constexpr unsigned char MYSQL_TYPE_VARCHAR        = 0x0F;    // VARBINARY
		constexpr unsigned char MYSQL_TYPE_NEWDECIMAL     = 0xf6;    // 高精度DECIMAL
		constexpr unsigned char MYSQL_TYPE_JSON           = 0xF5;    // JSON (MySQL 5.7+)
	}
}



namespace mysql
{
	constexpr unsigned char PACKAGE_OK = 0x00;
	constexpr unsigned char PACKAGE_ERR = 0xFF;
	constexpr unsigned char PACKAGE_EOF = 0xFE;
}

namespace mysql
{
	struct FieldInfo
	{
		std::string name;
		unsigned char type;
		unsigned char decimals;
	};

	struct Result
	{
		std::vector<std::string> contents;
	};
}

namespace mysql
{
	struct OKResponse
	{
	public:
		unsigned int mAffectedRows = 0;
		unsigned int mLastInsertId = 0;
		unsigned int mServerStatus = 0;
		unsigned int mWarningCount = 0;
	};
}

namespace mysql
{
	class IResponse
	{
	public:
		virtual bool OnDecode(const std::string & message) = 0;
	};
}

namespace mysql
{
	class HandshakeResponse : public IResponse
	{
	public:
		bool OnDecode(const std::string &message) final;
	public:
		uint8_t protocol_version;    // 协议版本
		std::string server_version;  // 服务器版本
		uint32_t connection_id;      // 连接 ID
		std::string salt;            // 盐值 (20 字节)
		uint16_t capability_flags;   // 服务器能力标志
		uint8_t character_set;       // 字符集
		uint16_t status_flags;       // 服务器状态
		std::string auth_plugin_name; // 认证插件名称
	};
}

#endif //APP_MYSQLCOMMON_H
