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
		bool Insert(const std::string & table, json::r::Document &, std::string& sqlCommand);
		bool Replace(const std::string & table, json::r::Document &, std::string& sqlCommand);
		bool CreateIndex(const std::string & table, const std::string & field, bool unique, std::string &sql);
		bool Select(const pb::Message & message, const std::string & where, int limit, std::string & sqlCommand);
        bool Select(const std::string & table, const std::string & where, std::vector<std::string> & fields, int limit, std::string & sqlCommand);
        bool Delete(const std::string & table, const std::string & filter, int limit, std::string& sqlCommand);
        bool Update(const std::string & table, const std::string & filter, const std::string & update, int limit, std::string& sqlCommand);
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