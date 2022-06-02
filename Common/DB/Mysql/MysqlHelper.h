#pragma once

#include"XCode/XCode.h"
#include"Util/Guid.h"
#include"Protocol/s2s.pb.h"
#include"rapidjson/document.h"
#include"DB/Mysql/MysqlDefine.h"

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
		bool ToSqlCommand(const s2s::Mysql::Add& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Save& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Query& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Update& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const s2s::Mysql::Delete& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const std::string& table, const std::string& cmd, Message& message, std::string& sql);

	 private:
		bool WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue);

	 private:
		std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
	};
}