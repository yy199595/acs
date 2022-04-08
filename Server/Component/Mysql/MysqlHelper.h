#pragma once

#include"XCode/XCode.h"
#include"Util/Guid.h"
#include"rapidjson/document.h"
#include"DB/Mysql/MysqlDefine.h"
#include<Protocol/s2s.pb.h>
using namespace google::protobuf;
namespace Sentry
{
	class MysqlTaskAction;

	class MysqlTaskSource;

	class MysqlHelper
	{
	 public:
		MysqlHelper() = default;
	 public:
		bool StartConnect();
		MysqlClient* GetMysqlClient();
	 public:
		bool ToSqlCommand(const s2s::Mysql::Add& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Save& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Query& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Update& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Delete& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const std::string& table, const std::string& cmd, Message& message, std::string& sql);

	 private:
		bool WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue);

	 private:
		bool InitMysqlTable();
		MysqlClient* ConnectMysql();
		bool DropTable(const std::string& db);
	 private:
		std::string mMysqlIp;         //ip地址
		unsigned short mMysqlPort;     //端口号
		std::string mDataBaseUser;     //用户名
		std::string mDataBasePasswd; //密码
		MysqlClient* mMysqlSocket;
		std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
		std::unordered_map<std::string, std::string> mSqlProtoMap;
		std::unordered_map<std::string, std::string> mSqlTableMap;
		std::unordered_map<std::thread::id, MysqlClient*> mMysqlSocketMap; //线程id和 socket
	};
}