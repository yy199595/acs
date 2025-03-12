#pragma once

#include<sstream>
#include"Message/s2s/db.pb.h"
#include"Proto/Include/Message.h"
#include"Yyjson/Document/Document.h"
namespace acs
{
	class SqlHelper
	{
	 public:
		SqlHelper() = default;
	 public:
		bool Insert(const db::sql::insert & request, std::string& sql);
		bool Replace(const db::sql::save & request, std::string& sql);
		bool CreateIndex(const db::sql::index & request, std::string &sql);
        bool Select(const db::sql::query::request & request, std::string & sql);
        bool Delete(const db::sql::del & request, std::string& sql);
        bool Update(const db::sql::update & request, std::string& sql);
		bool FindPage(const db::sql::query::page & request, std::string& sql);
        bool ToSqlCommand(const std::string& table, const std::string& cmd, json::r::Document& message, std::string& sql);
		bool Create(const std::string & tab, const std::string & message, const std::vector<std::string> & keys, std::string & sql);
		bool Create(const std::string & tab, const pb::Message & message, const std::vector<std::string> & keys, std::string & sql);
    private:
        bool WriterToStream(std::stringstream& stream, const json::r::Value& jsonValue);
    private:
        std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
	};
}