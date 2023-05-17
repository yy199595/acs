#include"SqlHelper.h"
#include"Entity/Actor/App.h"
#include"google/protobuf/util/json_util.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
namespace Tendo
{
    void SqlHelper::GetFiles(const Message &message, std::stringstream &ss, char cc)
    {
        const Descriptor * descriptor = message.GetDescriptor();
        for(int index = 0; index < descriptor->field_count(); index++)
        {
            const FieldDescriptor * fieldDescriptor = descriptor->field(index);
            if(index != descriptor->field_count() - 1)
            {
                ss << fieldDescriptor->name() << cc;
                continue;
            }
            ss << fieldDescriptor->name();
        }
    }

	bool SqlHelper::ToSqlCommand(const std::string& table, const std::string& cmd,
		const Message& message, std::string& sql)
    {
        this->mSqlCommandStream.str("");
        this->mSqlCommandStream << cmd << " into " << table << '(';

        this->GetFiles(message, this->mSqlCommandStream);

        this->mSqlCommandStream << ")values(";
        const Reflection *pReflection = message.GetReflection();
        const Descriptor * descriptor = message.GetDescriptor();
        for(int index = 0; index < descriptor->field_count(); index++)
        {
            const FieldDescriptor * fieldDesc = descriptor->field(index);
            if(fieldDesc->is_repeated())
            {
                return false;
            }
            switch (fieldDesc->type())
            {
                case FieldDescriptor::Type::TYPE_STRING:
                case FieldDescriptor::Type::TYPE_BYTES:
                    mSqlCommandStream << "'" << pReflection->GetString(message, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_INT32:
                    mSqlCommandStream << pReflection->GetInt32(message, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_UINT32:
                    mSqlCommandStream << pReflection->GetUInt32(message, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_INT64:
                    mSqlCommandStream << pReflection->GetInt64(message, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_UINT64:
                    mSqlCommandStream << pReflection->GetUInt64(message, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_FLOAT:
                    mSqlCommandStream << pReflection->GetFloat(message, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_DOUBLE:
                    mSqlCommandStream << pReflection->GetDouble(message, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_MESSAGE:
                {
                    std::string json;
#ifdef __OS_WIN__
#undef GetMessage //
#endif
                    const Message & value = pReflection->GetMessage(message, fieldDesc);
                    if(util::MessageToJsonString(value, &json).ok())
                    {
                        mSqlCommandStream << "'" << json << "',";
                    }
                    break;
                }
				case FieldDescriptor::Type::TYPE_BOOL:
				{
					int value = pReflection->GetBool(message, fieldDesc) ? 1 : 0;
					this->mSqlCommandStream << value << ",";
					break;
				}
                default:
					CONSOLE_LOG_ERROR("unknown type [" << fieldDesc->type_name() << "]" << fieldDesc->full_name());
                    return false;
            }
        }
        sql = mSqlCommandStream.str();
        sql[sql.size() - 1] = ')';
        return true;
    }

	bool SqlHelper::Update(const std::string & table,
		const std::string & where, const std::string & update, std::string& sqlCommand)
	{
        if(!this->Parse(this->mDocument1, where)
            || !this->Parse(this->mDocument2, update))
        {
            return false;
        }

		size_t index = 0;
		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "update " << table << " set ";
		auto iter1 = this->mDocument2.MemberBegin();
		for (; iter1 != this->mDocument2.MemberEnd(); iter1++, index++)
		{
			const rapidjson::Value& jsonValue = iter1->value;
			this->mSqlCommandStream << iter1->name.GetString() << "=";
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < this->mDocument2.MemberCount())
			{
				this->mSqlCommandStream << ",";
			}
		}
		index = 0;
		this->mSqlCommandStream << " where ";
		auto iter2 = this->mDocument1.MemberBegin();
		for (; iter2 != this->mDocument1.MemberEnd(); iter2++, index++)
		{
			const rapidjson::Value& jsonValue = iter2->value;
			this->mSqlCommandStream << iter2->name.GetString() << "=";
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < this->mDocument1.MemberCount())
			{
				this->mSqlCommandStream << " and ";
			}
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}

    bool SqlHelper::GetValue(rapidjson::Document &document, const std::string &key, std::string &value)
    {
        if(!key.empty() && document.HasMember(key.c_str()))
        {
            rapidjson::Value & jsonValue = document[key.c_str()];
            if(jsonValue.IsString())
            {
                value.append(jsonValue.GetString(), jsonValue.GetStringLength());
                return true;
            }
            if(jsonValue.IsInt() || jsonValue.IsUint())
            {
                value = std::to_string(jsonValue.GetInt());
                return true;
            }
            if(jsonValue.IsInt64() || jsonValue.IsUint64())
            {
                value = std::to_string(jsonValue.GetInt64());
                return true;
            }
        }
        return false;
    }

    bool SqlHelper::Parse(rapidjson::Document &doc, const std::string &json)
    {
        const char * str = json.c_str();
        const size_t size = json.size();
        return !doc.Parse(str, size).HasParseError() && doc.IsObject();
    }

    bool SqlHelper::GetValue(const std::string &key, std::string &value)
    {
        return this->GetValue(this->mDocument1, key, value);
    }

	bool SqlHelper::WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue)
	{
		if (jsonValue.IsString())
		{
			const char* str = jsonValue.GetString();
			size_t size = jsonValue.GetStringLength();
			this->mSqlCommandStream << "'";
			this->mSqlCommandStream.write(str, size);
			this->mSqlCommandStream << "'";
			return true;
		}
		if (jsonValue.IsInt())
		{
			int number = jsonValue.GetInt();
			this->mSqlCommandStream << number;
			return true;
		}
		if (jsonValue.IsInt64())
		{
			long long number = jsonValue.GetInt64();
			this->mSqlCommandStream << number;
			return true;
		}
		if (jsonValue.IsFloat())
		{
			float number = jsonValue.GetFloat();
			this->mSqlCommandStream << number;
			return true;
		}
		if (jsonValue.IsInt64())
		{
			double number = jsonValue.GetDouble();
			this->mSqlCommandStream << number;
			return true;
		}
        if(jsonValue.IsBool())
        {
            bool value = jsonValue.GetBool();
            this->mSqlCommandStream << value ? 1 : 0;
            return true;
        }
        if(jsonValue.IsObject())
        {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            jsonValue.Accept(writer);
            this->mSqlCommandStream << "'";
            const char * str = buffer.GetString();
            const size_t size = buffer.GetLength();
            this->mSqlCommandStream.write(str, size);
            this->mSqlCommandStream << "'";
            return true;
        }
		return false;
	}

	bool SqlHelper::Delete(const std::string & table, const std::string & where, std::string& sqlCommand)
	{
        if(!this->Parse(this->mDocument1, where))
        {
            return false;
        }

		size_t index = 0;
		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "delete from " << table << " where ";
		auto iter = this->mDocument1.MemberBegin();
		for (; iter != this->mDocument1.MemberEnd(); iter++, index++)
		{
			const char* key = iter->name.GetString();
			this->mSqlCommandStream << key << "=";
			rapidjson::Value& jsonValue = iter->value;
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < this->mDocument1.MemberCount())
			{
				this->mSqlCommandStream << " and ";
			}
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}

	bool SqlHelper::Insert(const Message & message, std::string& sqlCommand)
	{
		const std::string  table = message.GetTypeName();
		return this->ToSqlCommand(table, "insert", message, sqlCommand);
	}

	bool SqlHelper::Replace(const Message & message, std::string& sqlCommand)
	{
		const std::string  table = message.GetTypeName();
		return this->ToSqlCommand(table, "replace", message, sqlCommand);
	}

	bool SqlHelper::Select(const google::protobuf::Message& message,
		const std::string& where, int limit, std::string& sqlCommand)
	{
		std::vector<std::string> fields;
		const Descriptor * descriptor = message.GetDescriptor();
		for(int index = 0; index < descriptor->field_count(); index++)
		{
			const FieldDescriptor * fieldDescriptor = descriptor->field(index);
			if(fieldDescriptor != nullptr)
			{
				fields.emplace_back(fieldDescriptor->name());
			}
		}
		const std::string & table = message.GetTypeName();
		return this->Select(table, where, fields, limit, sqlCommand);
	}

	bool SqlHelper::Select(const std::string & name, const std::string & where,
		std::vector<std::string> & fields, int limit, std::string & sqlCommand)
	{
		if (fields.empty())
		{
			return false;
		}
		this->mSqlCommandStream.str("");
		if (!this->Parse(this->mDocument1, where))
		{
			return false;
		}
		this->mSqlCommandStream << "SELECT ";
		for (int index = 0; index < fields.size(); index++)
		{
			const std::string& field = fields.at(index);
			this->mSqlCommandStream << field;
			if (index != fields.size() - 1)
			{
				this->mSqlCommandStream << ",";
			}
		}
		this->mSqlCommandStream << " from " << name;
		if (this->mDocument1.MemberCount() == 0)
		{
			sqlCommand = this->mSqlCommandStream.str();
			return true;
		}
		size_t index = 0;
		this->mSqlCommandStream << " where ";
		auto iter = this->mDocument1.MemberBegin();
		for (; iter != this->mDocument1.MemberEnd(); iter++, index++)
		{
			const char* key = iter->name.GetString();
			const rapidjson::Value& jsonValue = iter->value;
			switch (key[0])
			{
			case '$': // >
				this->mSqlCommandStream << key + 1 << ">";
				break;
			case '%': // <
				this->mSqlCommandStream << key + 1 << "<";
				break;
			case '&': //>=
				this->mSqlCommandStream << key + 1 << ">=";
				break;
			case '*': //<=
				this->mSqlCommandStream << key + 1 << "<=";
				break;
			default:
				this->mSqlCommandStream << key << "=";
				break;
			}
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < this->mDocument1.MemberCount())
			{
				this->mSqlCommandStream << " and ";
			}
		}
		if (limit != 0)
		{
			this->mSqlCommandStream << " limit " << limit;
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}

	bool SqlHelper::Create(const google::protobuf::Message& message, const std::string& tab, const std::vector<std::string>& keys, std::string& sql)
	{
		const google::protobuf::Descriptor* descriptor = message.GetDescriptor();

		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "CREATE TABLE IF NOT EXISTS  " << tab << "(";
		for (int index = 0; index < descriptor->field_count(); index++)
		{
			const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->field(index);
			this->mSqlCommandStream << fieldDescriptor->name();
			switch (fieldDescriptor->cpp_type())
			{
				case FieldDescriptor::Type::TYPE_INT32:
				case FieldDescriptor::Type::TYPE_UINT32:
					this->mSqlCommandStream << " INT(20) NOT NULL DEFAULT 0,";
					break;
				case FieldDescriptor::Type::TYPE_INT64:
				case FieldDescriptor::Type::TYPE_UINT64:
					this->mSqlCommandStream << " BIGINT(32) NOT NULL DEFAULT 0,";
					break;
				case FieldDescriptor::Type::TYPE_FLOAT:
				case FieldDescriptor::Type::TYPE_DOUBLE:
					this->mSqlCommandStream << " DOUBLE(32) NOT NULL DEFAULT 0";
					break;
				case FieldDescriptor::Type::TYPE_STRING:
					this->mSqlCommandStream << " VARCHAR(64) NOT NULL DEFAULT ''";
					break;
				case FieldDescriptor::Type::TYPE_BYTES:
					this->mSqlCommandStream << " BLOB(64) NOT NULL DEFAULT ''";
					break;
				case FieldDescriptor::Type::TYPE_BOOL:
					this->mSqlCommandStream << " BOOLEAN NOT NULL DEFAULT 0";
					break;
				case FieldDescriptor::TYPE_MESSAGE:
					this->mSqlCommandStream << " JSON";
					break;
				default:
					return false;
			}
			if(index != descriptor->field_count() - 1)
			{
				this->mSqlCommandStream << ",";
			}
		}

		if(!keys.empty())
		{
			this->mSqlCommandStream << ",PRIMARY KEY (";
			for (size_t index = 0; index < keys.size(); index++)
			{
				const std::string& key = keys.at(index);
				if (descriptor->FindFieldByName(key) == nullptr)
				{
					return false;
				}
				this->mSqlCommandStream << "`" << key << "`";
				if(index < keys.size() - 1)
				{
					this->mSqlCommandStream << ",";
				}
			}
			this->mSqlCommandStream << ")";
		}
		else
		{
			this->mSqlCommandStream << ")";
		}
		this->mSqlCommandStream << ")";
		sql = this->mSqlCommandStream.str();
		return true;
	}
}