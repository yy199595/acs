#include"MysqlHelper.h"
#include"App/App.h"
#include"Component/ProtoComponent.h"
#include"google/protobuf/util/json_util.h"
namespace Sentry
{
    MysqlHelper::MysqlHelper(ProtoComponent *component)
    {
        this->mPorotComponent = component;
    }

    void MysqlHelper::GetFiles(const Message &message, std::stringstream &ss, char cc)
    {
        this->mFieldList.clear();
        const Reflection *pReflection = message.GetReflection();
        pReflection->ListFields(message, &this->mFieldList);
        for (size_t index = 0; index < this->mFieldList.size(); index++)
        {
            if(index != this->mFieldList.size() - 1)
            {
                ss << this->mFieldList[index]->name() << cc;
                continue;
            }
            ss << this->mFieldList[index]->name();
        }
    }

	bool MysqlHelper::ToSqlCommand(const std::string& table, const std::string& cmd,
		Message& message, std::string& sql)
    {
        this->mSqlCommandStream.str("");
        this->mSqlCommandStream << cmd << " into " << table << '(';

        this->GetFiles(message, this->mSqlCommandStream);
        
        this->mSqlCommandStream << ")values(";
        const Reflection *pReflection = message.GetReflection();
        for (const FieldDescriptor * fieldDesc: this->mFieldList)
        {
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
                default:
                    return false;
            }
        }
        sql = mSqlCommandStream.str();
        sql[sql.size() - 1] = ')';
        return true;
    }

	bool MysqlHelper::ToSqlCommand(const db::mysql::update& messageData, std::string& sqlCommand)
	{
        const std::string & json1 = messageData.where_json();
        const std::string & json2 = messageData.update_json();
        if(!this->Parse(this->mDocument1, json1)
            || !this->Parse(this->mDocument2, json2))
        {
            return false;
        }

		size_t index = 0;
		this->mSqlCommandStream.str("");
		const std::string& table = messageData.table();
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

    bool MysqlHelper::GetValue(rapidjson::Document &document, const std::string &key, std::string &value)
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

    bool MysqlHelper::Parse(rapidjson::Document &doc, const std::string &json)
    {
        const char * str = json.c_str();
        const size_t size = json.size();
        return !doc.Parse(str, size).HasParseError() && doc.IsObject();
    }

    bool MysqlHelper::GetValue(const std::string &key, std::string &value)
    {
        return this->GetValue(this->mDocument1, key, value);
    }

	bool MysqlHelper::WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue)
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

	bool MysqlHelper::ToSqlCommand(const db::mysql::remove& messageData, std::string& sqlCommand)
	{
		const std::string& json = messageData.where_json();
        if(!this->Parse(this->mDocument1, json))
        {
            return false;
        }

		size_t index = 0;
		this->mSqlCommandStream.str("");
		const std::string& table = messageData.table();
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

	bool MysqlHelper::ToSqlCommand(const db::mysql::add& request, std::string& sqlCommand)
	{
        LOG_CHECK_RET_FALSE(this->mPorotComponent);
		this->mMessage = this->mPorotComponent->New(request.data());
		if (this->mMessage == nullptr)
		{
			return false;
		}
		const std::string& table = request.table();
		return this->ToSqlCommand(table, "insert", *this->mMessage, sqlCommand);
	}

	bool MysqlHelper::ToSqlCommand(const db::mysql::save& request, std::string& sqlCommand)
	{
        LOG_CHECK_RET_FALSE(this->mPorotComponent);
		this->mMessage = this->mPorotComponent->New(request.data());
		if (this->mMessage == nullptr)
		{
			return false;
		}
		const std::string& table = request.table();
		return this->ToSqlCommand(table, "replace", *this->mMessage, sqlCommand);
	}

	bool MysqlHelper::ToSqlCommand(const db::mysql::query& request, std::string& sqlCommand)
	{
		this->mSqlCommandStream.str("");
		const std::string& json = request.where_json();
        if(!this->Parse(this->mDocument1, json))
        {
            return false;
        }

		this->mSqlCommandStream << "select * from " << request.table();
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
			switch(key[0])
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
        if(request.limit() != 0)
        {
            this->mSqlCommandStream << " limit " << request.limit();
        }
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}
}