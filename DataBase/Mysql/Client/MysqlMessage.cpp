//
// Created by zmhy0073 on 2022/8/25.
//

#include"MysqlMessage.h"
#include"errmsg.h"
#include"spdlog/fmt/fmt.h"
#include"sstream"
#include"Guid/Guid.h"
#include"Log/CommonLogDef.h"
#include"Component/LoggerComponent.h"
#include"google/protobuf/util/json_util.h"
using namespace Sentry;
namespace Mysql
{
    ICommand::ICommand()
    {
        this->mRpcId = Helper::Guid::Create();
    }
}

namespace Mysql
{
    bool PingCommand::Invoke(MYSQL * sock, std::string &error)
    {
        switch(mysql_ping(sock))
        {
            case 0: return true;
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
        return false;
    }

}

namespace Mysql
{
    SqlCommand::SqlCommand(const std::string &sql)
        : mSql(std::move(sql))
    {

    }

    bool SqlCommand::Invoke(MYSQL * sock, std::string &error)
    {
        if (mysql_real_query(sock, this->mSql.c_str(), this->mSql.size()) != 0)
        {
            error = mysql_error(sock);
            return false;
        }
        return true;
    }

    QueryCommand::QueryCommand(const std::string &sql)
        : mSql(std::move(sql))
    {

    }

    bool QueryCommand::Invoke(MYSQL * sock, std::string &error)
    {
        if (mysql_real_query(sock, this->mSql.c_str(), this->mSql.size()) != 0)
        {
            error = mysql_error(sock);
            return false;
        }
        MYSQL_RES *result1 = mysql_store_result(sock);
        if(result1 == nullptr)
        {
            return false;
        }
        while(MYSQL_ROW row = mysql_fetch_row(result1))
        {
            std::string json;
            rapidjson::Document * jsonDocument =
                new rapidjson::Document(rapidjson::kObjectType);

            unsigned int fieldCount = mysql_field_count(sock);
            unsigned long* lengths = mysql_fetch_lengths(result1);
            for (unsigned int index = 0; index < fieldCount; index++)
            {
                st_mysql_field* field = mysql_fetch_field(result1);
                this->Write(*jsonDocument, field, row[index], lengths[index]);
            }
            this->emplace_back(std::move(jsonDocument));
        }
        mysql_free_result(result1);
        return true;
    }

    bool QueryCommand::Write(rapidjson::Document &document, st_mysql_field *filed, const char *str, int len)
    {
        if(str == nullptr || len == 0)
        {
            return true;
        }
        rapidjson::GenericStringRef<char> key(filed->name);
        switch(filed->type)
        {
            case enum_field_types::MYSQL_TYPE_TINY:
            {
                int value = std::atol(str);
                document.AddMember(key, value, document.GetAllocator());
            }
                return true;
            case enum_field_types::MYSQL_TYPE_LONG:
            case enum_field_types::MYSQL_TYPE_LONGLONG:
            {
                long long value = std::atoll(str);
                document.AddMember(key, value, document.GetAllocator());
            }
                return true;
            case enum_field_types::MYSQL_TYPE_FLOAT:
            case enum_field_types::MYSQL_TYPE_DOUBLE:
            {
                double value = std::atof(str);
                document.AddMember(key, value, document.GetAllocator());
            }
                return true;
            case enum_field_types::MYSQL_TYPE_BLOB:
            case enum_field_types::MYSQL_TYPE_STRING:
            case enum_field_types::MYSQL_TYPE_VARCHAR:
            case enum_field_types::MYSQL_TYPE_VAR_STRING:
            {
                rapidjson::Value value(str, len);
                document.AddMember(key, value, document.GetAllocator());
            }
                return true;
            case enum_field_types::MYSQL_TYPE_JSON:
            {
                rapidjson::Document doc;
                if(doc.Parse(str, len).HasParseError())
                {
                    return false;
                }
                document.AddMember(key, doc, document.GetAllocator());
            }
                break;
        }
        CONSOLE_LOG_ERROR(filed->name << " : " << (int)filed->type);
        return false;
    }
}

namespace Mysql
{
    CreateTabCommand::CreateTabCommand(std::shared_ptr<google::protobuf::Message> message, std::vector<std::string> & keys)
        : mMessage(message), mKeys(keys)
    {

    }

    bool CreateTabCommand::Invoke(MYSQL* sock, std::string& error)
    {
        std::string name = this->mMessage->GetTypeName();
        const size_t pos = name.find('.');
        if (pos == std::string::npos)
        {
            error = "proto name not xxx.xxx";
            CONSOLE_LOG_ERROR(error);
            return false;
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
                CONSOLE_LOG_ERROR(error);
                return false;
            }
            if (mysql_select_db(sock, db.c_str()) != 0)
            {
                error = mysql_error(sock);
                CONSOLE_LOG_ERROR(error);
                return false;
            }
        }
        if (mysql_query(sock, "SHOW TABLES") != 0)
        {
            error = mysql_error(sock);
            CONSOLE_LOG_ERROR(error);
            return false;
        }
        std::set<std::string> tables;
        MYSQL_RES * result1 = mysql_store_result(sock);
        while(MYSQL_ROW row = mysql_fetch_row(result1))
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
            if (!this->CreateTable(sock, tab, error))
            {              
                return false;
            }
            return true;
        }
        return this->CheckTableField(sock, tab, error);
    }

    void CreateTabCommand::ClearBuffer()
    {
        this->mBuffer.clear();
        this->mBuffer.str("");
    }

    bool CreateTabCommand::CheckTableField(MYSQL *sock, const std::string &tab, std::string &error)
    {
        this->ClearBuffer();
        this->mBuffer << "DESC " << tab << ';';
        const std::string sql = this->mBuffer.str();
        if (mysql_query(sock, sql.c_str()) != 0)
        {
            error = mysql_error(sock);
            CONSOLE_LOG_ERROR(error);
            return false;
        }

        std::set<std::string> tableFileds;
        MYSQL_RES * result1 = mysql_store_result(sock);
        while(MYSQL_ROW row = mysql_fetch_row(result1))
        {
            unsigned int fieldCount = mysql_field_count(sock);
            unsigned long *lengths = mysql_fetch_lengths(result1);
            if (fieldCount > 0 && lengths != nullptr)
            {
                tableFileds.emplace(std::string(row[0], lengths[0]));
            }
        }
        mysql_free_result(result1);
        const Descriptor* descriptor = this->mMessage->GetDescriptor();
        for (int index = 0; index < descriptor->field_count(); index++)
        {
            const FieldDescriptor *fileDescriptor = descriptor->field(index);
            if (fileDescriptor == nullptr)
            {
                error = fmt::format("proto field error index = {0}", index);
                return false;
            }
            if(tableFileds.find(fileDescriptor->name()) == tableFileds.end()) //没有这个字段
            {
                if(!this->AddNewField(sock, tab, fileDescriptor->name(), error))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool CreateTabCommand::AddNewField(MYSQL *sock, const std::string &tab,
                                       const std::string &field, std::string &error)
    {
        const FieldDescriptor * fieldDescriptor = this->mMessage->GetDescriptor()->FindFieldByName(field);
        if(fieldDescriptor == nullptr)
        {
            error = fmt::format("not find filed : {0}", field);
            CONSOLE_LOG_ERROR(error);
            return false;
        }
        this->ClearBuffer();
        this->mBuffer << "ALTER TABLE " << tab << " ADD ";
        if(!this->ForeachMessage(fieldDescriptor))
        {
            error = fmt::format("field error : {0}", field);
            CONSOLE_LOG_ERROR(error);
            return false;
        }
        const std::string sql = this->mBuffer.str();
        if(mysql_query(sock, sql.c_str()) != 0)
        {
            error = mysql_error(sock);
            CONSOLE_LOG_ERROR(error);
            return false;
        }
        return true;
    }

    bool CreateTabCommand::CreateTable(MYSQL* sock, const std::string & tab, std::string & error)
    {
        this->ClearBuffer();
        this->mBuffer << "CREATE TABLE `" << tab << "`(";
        const Descriptor *descriptor = this->mMessage->GetDescriptor();
        for (int index = 0; index < descriptor->field_count(); index++)
        {
            const FieldDescriptor *fileDescriptor = descriptor->field(index);
            if (fileDescriptor == nullptr)
            {
                error = fmt::format("proto field error index = {0}", index);
                CONSOLE_LOG_ERROR(error);
                return false;
            }
            if (!this->ForeachMessage(fileDescriptor))
            {
                error = fmt::format("proto field error field = {0}", fileDescriptor->name());
                CONSOLE_LOG_ERROR(error);
                return false;
            }
            if (index != descriptor->field_count() - 1)
            {
                this->mBuffer << ',';
            }
        }
        if(!this->mKeys.empty())
        {
            this->mBuffer << ", PRIMARY KEY (";
            for (size_t index = 0; index < this->mKeys.size(); index++)
            {
                this->mBuffer << '`' << this->mKeys[index] << '`';
                if(index != this->mKeys.size() - 1)
                {
                    this->mBuffer << ',';
                }
            }
            this->mBuffer << ')';
        }
        this->mBuffer << ") ENGINE=InnoDB DEFAULT CHARSET=utf8;";
        const std::string sql = this->mBuffer.str();
        if (mysql_real_query(sock, sql.c_str(), sql.length()) != 0)
        {
            error = mysql_error(sock);
            CONSOLE_LOG_ERROR(error);
            return false;
        }
        return true;
    }

    bool CreateTabCommand::ForeachMessage(const FieldDescriptor *field)
    {
        const Message & message = *this->mMessage;
        this->mBuffer << "`" << field->name() << "` ";
        const Reflection * reflection = this->mMessage->GetReflection();
        switch(field->type())
        {
            case FieldDescriptor::TYPE_INT32:
            {
                int value = reflection->GetInt32(message, field);
                this->mBuffer << "INT(20) NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_UINT32:
            {
                unsigned int value = reflection->GetUInt32(message, field);
                this->mBuffer << "INT(20) NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_UINT64:
            {
                unsigned long long value = reflection->GetUInt64(message, field);
                this->mBuffer << "BIGINT(32) NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_INT64:
            {
                long long value = reflection->GetInt64(message, field);
                this->mBuffer << "BIGINT(32) NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_FLOAT:
            {
                float value = reflection->GetFloat(message, field);
                this->mBuffer << "FLOAT(20) NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_DOUBLE:
            {
                double value = reflection->GetDouble(message, field);
                this->mBuffer << "DOUBLE(32) NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_STRING:
            {
                std::string value = reflection->GetString(message, field);
                this->mBuffer << "VARCHAR(64) NOT NULL DEFAULT '" << value << "'";
            }
                return true;
            case FieldDescriptor::TYPE_BYTES:
            {
                std::string value = reflection->GetString(message, field);
                this->mBuffer << "BLOB(64) NOT NULL DEFAULT '" << value << "'";
            }
                return true;
            case FieldDescriptor::TYPE_BOOL:
            {
                bool value = reflection->GetBool(message, field);
                this->mBuffer << "BOOLEAN NOT NULL DEFAULT " << value;
            }
                return true;
            case FieldDescriptor::TYPE_MESSAGE:
            {
                std::string json;
                const Message & message = reflection->GetMessage(message, field);
                if(util::MessageToJsonString(message, &json).ok())
                {
                    this->mBuffer << "JSON NOT NULL DEFAULT " << json;
                    return true;
                }
            }
        }
        return false;
    }
}

namespace Mysql
{
    SetMainKeyCommand::SetMainKeyCommand(const std::string &tab, std::vector<std::string> &keys)
        : mKeys(keys), mTable(tab)
    {

    }

    bool SetMainKeyCommand::Invoke(MYSQL * sock, std::string &error)
    {
        return false;
    }
}