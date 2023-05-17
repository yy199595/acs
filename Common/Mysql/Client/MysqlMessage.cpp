//
// Created by zmhy0073 on 2022/8/25.
//
#ifdef __ENABLE_MYSQL__
#include"MysqlMessage.h"

#include <utility>
#include"errmsg.h"
#include"spdlog/fmt/fmt.h"
#include"sstream"
#include"Util/Guid/Guid.h"
#include"Entity/Actor/App.h"
#include"Util/Json/Lua/Json.h"
#include"google/protobuf/util/json_util.h"

namespace Mysql
{
	void Response::SetError(const char* str)
	{
		this->mError = str;
	}

    int Response::WriteToLua(lua_State* lua) const
	{
		lua_pushboolean(lua, this->IsOk());
		if (this->mResults.empty())
		{
			return 1;
		}
		lua_createtable(lua, 0, this->mResults.size());
		for (int index = 0; index < this->mResults.size(); index++)
		{
			lua_pushinteger(lua, index + 1);
			Lua::RapidJson::Write(lua, this->mResults[0]);
			lua_settable(lua, -3);
		}
		return 2;
	}

	void Response::Add(const std::string& json)
	{
		this->mResults.emplace_back(json);
	}

	void Response::Add(const char* str, size_t len)
	{
		this->mResults.emplace_back(str, len);
	}
}

namespace Mysql
{
    bool PingCommand::Invoke(MYSQL* mysql, std::shared_ptr<Response> & response)
	{
		switch (mysql_ping(mysql))
		{
		case 0:
			return true;
		case CR_UNKNOWN_ERROR:
			response->SetError("CR_UNKNOWN_ERROR");
			return false;
		case CR_SERVER_GONE_ERROR:
			response->SetError("CR_SERVER_GONE_ERROR");
			return false;
		case CR_COMMANDS_OUT_OF_SYNC:
			response->SetError("CR_COMMANDS_OUT_OF_SYNC");
			return false;
		}
		return true;
	}

}

namespace Mysql
{
    SqlCommand::SqlCommand(const std::string &sql)
        : mSql(std::move(sql))
    {

    }

    bool SqlCommand::Invoke(MYSQL* mysql, std::shared_ptr<Response> & response)
    {
		const char * sql = this->mSql.c_str();
		const size_t length = this->mSql.size();
        if (mysql_real_query(mysql, sql, length) != 0)
        {
			response->SetError(mysql_error(mysql));
            return false;
        }
        return true;
    }

    QueryCommand::QueryCommand(const std::string &sql)
        : mSql(sql)
    {

    }

    bool QueryCommand::Invoke(MYSQL * sock, std::shared_ptr<Response> & response)
    {
		const char * sql = this->mSql.c_str();
		const size_t length = this->mSql.size();
        if (mysql_real_query(sock, sql, length) != 0)
        {
			response->SetError(mysql_error(sock));
            return false;
        }
        MYSQL_RES *result1 = mysql_store_result(sock);
        if(result1 == nullptr)
        {
			response->SetError("query result is null");
            return false;
        }
		std::string json;
        while(MYSQL_ROW row = mysql_fetch_row(result1))
        {
			json.clear();
			size_t count = 0;
            Json::Writer jsonDocument;
            unsigned int fieldCount = mysql_field_count(sock);
            unsigned long* lengths = mysql_fetch_lengths(result1);
			for (unsigned int index = 0; index < fieldCount; index++)
            {
				st_mysql_field* field = mysql_fetch_field_direct(result1, index);
				if(QueryCommand::Write(jsonDocument, field, row[index], lengths[index]))
				{
					count++;
				}
            }
			if(count > 0)
			{
				jsonDocument.WriterStream(&json);
				response->Add(json.c_str(), json.size());
			}
        }
        mysql_free_result(result1);
        return true;
    }

    bool QueryCommand::Write(Json::Writer &document, st_mysql_field *filed, const char *str, int len)
    {
        if(str == nullptr || len == 0 || filed == nullptr)
        {
            return false;
        }
        switch(filed->type)
        {
            case enum_field_types::MYSQL_TYPE_TINY:
            {
                int value = std::atoi(str);
                document.Add(filed->name).Add(value);
                return true;
            }
            case enum_field_types::MYSQL_TYPE_LONG:
            case enum_field_types::MYSQL_TYPE_LONGLONG:
            {
                long long value = std::atoll(str);
                document.Add(filed->name).Add(value);
                return true;
            }
            case enum_field_types::MYSQL_TYPE_FLOAT:
            case enum_field_types::MYSQL_TYPE_DOUBLE:
            {
                double value = std::atof(str);
                document.Add(filed->name).Add(value);
                return true;
            }
            case enum_field_types::MYSQL_TYPE_BLOB:
            case enum_field_types::MYSQL_TYPE_STRING:
            case enum_field_types::MYSQL_TYPE_VARCHAR:
            case enum_field_types::MYSQL_TYPE_VAR_STRING:
            {
                //rapidjson::Value value(str, len);
                document.Add(filed->name).Add(str, len);
                return true;
            }
            case enum_field_types::MYSQL_TYPE_JSON:
            {
                rapidjson::Document doc;
                if(doc.Parse(str, len).HasParseError())
                {
                    return false;
                }
                document.Add(filed->name).Add(doc);
                return true;
            }
        }
        CONSOLE_LOG_ERROR(filed->name << " : " << (int)filed->type);
        return false;
    }
}

namespace Mysql
{
    CreateTabCommand::CreateTabCommand(std::string  tab,
			std::shared_ptr<google::protobuf::Message> message, std::vector<std::string> & keys)
        : mMessage(std::move(message)), mKeys(keys), mTable(std::move(tab))
    {

    }

    bool CreateTabCommand::Invoke(MYSQL* sock, std::shared_ptr<Response> & response)
    {
        const size_t pos = this->mTable.find('.');
        if (pos == std::string::npos)
        {
			response->SetError("proto name not xxx.xxx");
            return false;
        }
        std::string db = this->mTable.substr(0, pos);
        std::string tab = this->mTable.substr(pos + 1);
        if (mysql_select_db(sock, db.c_str()) != 0)
        {
            this->mBuffer << "CREATE DATABASE IF NOT EXISTS " << db;
            this->mBuffer << " DEFAULT CHARSET utf8 COLLATE utf8_general_ci;";

            const std::string sql = mBuffer.str();
            if (mysql_real_query(sock, sql.c_str(), sql.length()) != 0)
            {
				response->SetError(mysql_error(sock));
                return false;
            }
            if (mysql_select_db(sock, db.c_str()) != 0)
            {
				response->SetError(mysql_error(sock));
				return false;
            }
        }
        if (mysql_query(sock, "SHOW TABLES") != 0)
        {
			response->SetError(mysql_error(sock));
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
			std::string error;
            if (!this->CreateTable(sock, tab, error))
            {
				response->SetError(error.c_str());
                return false;
            }
            return true;
        }
		std::string error;
		if(!this->CheckTableField(sock, tab, error))
		{
			response->SetError(error.c_str());
			return false;
		}
		return true;
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
        const Message &message = *this->mMessage;
        this->mBuffer << "`" << field->name() << "` ";
        if (field->is_repeated())
        {
            return false;
        }
        switch (field->type())
        {
            case FieldDescriptor::TYPE_INT32:
                this->mBuffer << "INT(20) NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_UINT32:
                this->mBuffer << "INT(20) NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_UINT64:
                this->mBuffer << "BIGINT(32) NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_INT64:
                this->mBuffer << "BIGINT(32) NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_FLOAT:
                this->mBuffer << "FLOAT(20) NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_DOUBLE:
                this->mBuffer << "DOUBLE(32) NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_STRING:
                this->mBuffer << "VARCHAR(64) NOT NULL DEFAULT ''";
                return true;
            case FieldDescriptor::TYPE_BYTES:
                this->mBuffer << "BLOB(64) NOT NULL DEFAULT ''";
                return true;
            case FieldDescriptor::TYPE_BOOL:
                this->mBuffer << "BOOLEAN NOT NULL DEFAULT 0";
                return true;
            case FieldDescriptor::TYPE_MESSAGE:
                this->mBuffer << "JSON";
                return true;
            default:
                return false;
        }
        return false;
    }
}

namespace Mysql
{
    SetMainKeyCommand::SetMainKeyCommand(const std::string &tab, std::vector<std::string> &keys)
        : mTable(tab), mKeys(keys)
    {

    }

    bool SetMainKeyCommand::Invoke(MYSQL*, std::shared_ptr<Response>& response)
    {
        return false;
    }
}
#endif