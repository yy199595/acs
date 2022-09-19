#pragma once

#include"XCode/XCode.h"
#include"Util/Guid.h"
#include"Message/db.pb.h"
#include"rapidjson/document.h"
#include"MysqlDefine.h"

using namespace google::protobuf;
namespace Sentry
{
	class MysqlHelper
	{
	 public:
		MysqlHelper() = default;
	 public:
		bool ToSqlCommand(const db::mysql::add& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const db::mysql::save& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const db::mysql::query& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const db::mysql::update& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const db::mysql::remove& messageData, std::string& sqlCommand);

		bool ToSqlCommand(const std::string& table, const std::string& cmd, Message& message, std::string& sql);

	 private:
		bool WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue);

	 private:
		std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
	};
}