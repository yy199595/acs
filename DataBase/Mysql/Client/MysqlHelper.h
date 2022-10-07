#pragma once

#include"XCode/XCode.h"
#include"Guid/Guid.h"
#include"Message/db.pb.h"
#include"rapidjson/document.h"
#include"MysqlDefine.h"

using namespace google::protobuf;
namespace Sentry
{
    class ProtoComponent;
	class MysqlHelper
	{
	 public:
		MysqlHelper(ProtoComponent * component);
	 public:

        bool ToSqlCommand(const db::mysql::add& messageData, std::string& sqlCommand);

        bool ToSqlCommand(const db::mysql::save& messageData, std::string& sqlCommand);

        bool ToSqlCommand(const db::mysql::query& messageData, std::string& sqlCommand);

        bool ToSqlCommand(const db::mysql::remove& messageData, std::string& sqlCommand);

        bool ToSqlCommand(const db::mysql::update& messageData, std::string& sqlCommand);

        bool ToSqlCommand(const std::string& table, const std::string& cmd, Message& message, std::string& sql);

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
        ProtoComponent * mPorotComponent;
        std::shared_ptr<Message> mMessage;
        std::stringstream mSqlCommandStream;
		std::stringstream mSqlCommandStream2;
    };
}