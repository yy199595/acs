//
// Created by zmhy0073 on 2022/8/25.
//

#include"MysqlMessage.h"
#include"errmsg.h"
#include"spdlog/fmt/fmt.h"
#include"sstream"
namespace Mysql
{
    Response::Response(MYSQL_RES *result)
    {
        this->mResult = result;
    }

    Response::Response(const std::string &error)
        : mError(error)
    {
        this->mResult = nullptr;
    }

    Response::~Response()
    {
        if(this->mResult != nullptr)
        {
            mysql_free_result(this->mResult);
        }
    }
}

namespace Mysql
{
    MYSQL_RES *PingCommand::Invoke(MYSQL * sock, std::string &error)
    {
        switch(mysql_ping(sock))
        {
            case 0:
                break;
            case CR_UNKNOWN_ERROR:
                error = "CR_UNKNOWN_ERROR";
                break;
            case CR_SERVER_GONE_ERROR:
                error = "CR_SERVER_GONE_ERROR";
                break;
            case CR_COMMANDS_OUT_OF_SYNC:
                error = "CR_COMMANDS_OUT_OF_SYNC";
                break;
        }
        return nullptr;
    }

	MYSQL_RES *SqlCommand::Invoke(MYSQL*, std::string & error)
	{
		return nullptr;
	}

    MYSQL_RES *QueryCommand::Invoke(MYSQL *, std::string &error)
    {
        return nullptr;
    }
}

namespace Mysql
{
    CreateTabCommand::CreateTabCommand(std::shared_ptr<google::protobuf::Message> message, int id)
        : ICommand(id), mMessage(message)
    {

    }
    MYSQL_RES* CreateTabCommand::Invoke(MYSQL* sock, std::string& error)
    {
        std::string name = this->mMessage->GetTypeName();
        const size_t pos = name.find('.');
        if (pos == std::string::npos)
        {
            error = "proto name not xxx.xxx";
            return nullptr;
        }
        std::string db = name.substr(0, pos);
        std::string tab = name.substr(pos + 1);
        if (mysql_select_db(sock, db.c_str()) != 0)
        {
            this->mBuffer << "CREATE DATABASE IF NOT EXISTS " << db;
            this->mBuffer << " DEFAULT CHARSET utf8 COLLATE utf8_general_ci;";

            const std::string sql = mBuffer.str();
            if (mysql_real_query(sock, sql.c_str(), sql.length()) != 0)
            {
                error = mysql_error(sock);
                return nullptr;
            }
            this->mBuffer.clear();
            this->mBuffer.str("");
            if (mysql_select_db(sock, db.c_str()) != 0)
            {
                error = mysql_error(sock);
                return nullptr;
            }
        }
        if (mysql_query(sock, "SHOW TABLES") != 0)
        {
            error = mysql_error(sock);
            return nullptr;
        }
        std::set<std::string> tables;
        MYSQL_RES * result1 = mysql_store_result(sock);

        MYSQL_ROW row = mysql_fetch_row(result1);
        if (row != nullptr)
        {
            unsigned int fieldCount = mysql_field_count(sock);
            unsigned long* lengths = mysql_fetch_lengths(result1);
            for (unsigned int index = 0; index < fieldCount; index++)
            {
                st_mysql_field* field = mysql_fetch_field(result1);
                tables.emplace(std::string(row[index], lengths[index]));
            }
        }
        mysql_free_result(result1);
        if (tables.find(tab) == tables.end())
        {
            if (!this->CreateTable(sock, error))
            {              
                return nullptr;
            }
        }
        else
        {

        }

        return nullptr;
    }

    bool CreateTabCommand::CreateTable(MYSQL* sock, std::string & error)
    {
        const Descriptor* descriptor = this->mMessage->GetDescriptor();
        for (int index = 0; index < descriptor->field_count(); index++)
        {
            const FieldDescriptor* fileDescriptor = descriptor->field(index);
            if (fileDescriptor == nullptr)
            {
                error = fmt::format("proto field error index = {0}", index);
                return false;
            }
            if (!this->ForeachMessage(fileDescriptor))
            {
                error = fmt::format("proto field error field = {0}", fileDescriptor->name());
                return false;
            }
        }
        std::string sql = this->mBuffer.str();
        if (mysql_real_query(sock, sql.c_str(), sql.length()) != 0)
        {
            error = mysql_error(sock);
            return false;
        }
        return true;
    }

    bool CreateTabCommand::ForeachMessage(const FieldDescriptor *field)
    {
        const Message & message = *this->mMessage;
        const Reflection * reflection = this->mMessage->GetReflection();
        switch(field->type())
        {
            case FieldDescriptor::TYPE_INT32:
            {
                int value = reflection->GetInt32(message, field);
            }
                return true;
            case FieldDescriptor::TYPE_UINT32:
            {
                int value = reflection->GetInt32(message, field);
            }
                return true;
            case FieldDescriptor::TYPE_UINT64:
            {
                int value = reflection->GetInt32(message, field);
            }
                return true;
            case FieldDescriptor::TYPE_INT64:

                break;
            case FieldDescriptor::TYPE_FLOAT:

                break;
            case FieldDescriptor::TYPE_DOUBLE:

                break;
            case FieldDescriptor::TYPE_STRING:

                break;
            case FieldDescriptor::TYPE_BYTES:

                break;
            case FieldDescriptor::TYPE_BOOL:

                break;
        }
        return false;
    }
}