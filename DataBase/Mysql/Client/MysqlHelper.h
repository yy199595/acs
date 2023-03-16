#pragma once

#include"XCode/XCode.h"
#include"Guid/Guid.h"
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

        bool Insert(const Message &, std::string& sqlCommand);
		bool Replace(const Message &, std::string& sqlCommand);

		bool Select(const Message & message, const std::string & where,
			int limit, std::string & sqlCommand);

        bool Select(const std::string & table, const std::string & where,
			std::vector<std::string> & fields, int limit, std::string & sqlCommand);

        bool Delete(const std::string & table, const std::string & where, std::string& sqlCommand);

        bool Update(const std::string & table, const std::string & where, const std::string & update, std::string& sqlCommand);

        bool ToSqlCommand(const std::string& table, const std::string& cmd, const Message& message, std::string& sql);

    public:
        bool GetValue(const std::string & key, std::string & value);
        std::shared_ptr<Message> GetData() { return std::move(this->mMessage); }
        bool GetValue(rapidjson::Document & document, const std::string & key, std::string & value);
    private:
        bool Parse(rapidjson::Document & doc, const std::string & json);
        void GetFiles(const Message & message, std::stringstream & ss, char cc = ',');
        bool WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue);

    private:
        rapidjson::Document mDocument1;
        rapidjson::Document mDocument2;
        std::shared_ptr<Message> mMessage;
        std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
    };
}