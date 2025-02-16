#include"SqlHelper.h"
#include"Entity/Actor/App.h"
#include"google/protobuf/util/json_util.h"

namespace acs
{
	bool SqlHelper::ToSqlCommand(const std::string& table, const std::string& cmd,
			json::r::Document & message, std::string& sql)
    {
		std::vector<const char * > keys;
		if(message.GetKeys(keys) <= 0)
		{
			return false;
		}
		size_t index = 0;
		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << cmd << " INTO " << table << '(';
		for(const char * key : keys)
		{
			index++;
			this->mSqlCommandStream  << key;
			if(index < keys.size())
			{
				this->mSqlCommandStream << ",";
			}
		}

		this->mSqlCommandStream << ")VALUES(";
		std::unique_ptr<json::r::Value> jsonValue;
		for(const char * key : keys)
		{
			message.Get(key, jsonValue);
			switch(jsonValue->GetType())
			{
				case YYJSON_TYPE_ARR:
				case YYJSON_TYPE_OBJ:
				{
					std::string json = jsonValue->ToString();
					mSqlCommandStream << "'" << json << "',";
					break;
				}
				case YYJSON_TYPE_STR:
				{
					std::string str;
					jsonValue->Get(str);
					mSqlCommandStream << "'" << str << "',";
					break;
				}
				case YYJSON_TYPE_NUM:
				{
					long long value = 0;
					jsonValue->Get(value);
					mSqlCommandStream << value << ",";
					break;
				}
				case YYJSON_TYPE_BOOL:
				{
					bool value = false;
					jsonValue->Get(value);
					mSqlCommandStream << (value ? "TRUE" : "FALSE") << ",";
					break;
				}
			}
		}
        sql = mSqlCommandStream.str();
        sql[sql.size() - 1] = ')';
        return true;
    }

	bool SqlHelper::Update(const std::string & table,
		const std::string & filter, const std::string & update, int limit, std::string& sqlCommand)
	{
		json::r::Document document1;
		json::r::Document document2;
		if(!document1.Decode(filter)|| !document2.Decode(update))
		{
			return false;
		}

		size_t index = 0;
		std::vector<const char *> keys;
		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "UPDATE " << table << " SET ";
		if(document2.GetKeys(keys) > 0)
		{
			std::unique_ptr<json::r::Value> value;
			for(const char * key : keys)
			{
				index++;
				if(document2.Get(key, value))
				{
					this->mSqlCommandStream << key << "=";
					this->WriterToStream(this->mSqlCommandStream, *value);
					if(index + 1 < keys.size())
					{
						this->mSqlCommandStream << ",";
					}
				}
			}
		}
		index = 0;
		keys.clear();
		if(document1.GetKeys(keys) > 0)
		{
			this->mSqlCommandStream << " WHERE ";
			std::unique_ptr<json::r::Value> value;
			for(const char * key : keys)
			{
				index++;
				if(document1.Get(key, value))
				{
					this->mSqlCommandStream << key << "=";
					this->WriterToStream(this->mSqlCommandStream, *value);
					if(index + 1 < keys.size())
					{
						this->mSqlCommandStream << " AND ";
					}
				}
			}
		}
		if(limit > 0)
		{
			this->mSqlCommandStream << " LIMIT " << limit;
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}

	bool SqlHelper::WriterToStream(std::stringstream& stream, const json::r::Value& jsonValue)
	{
		switch(jsonValue.GetType())
		{
			case YYJSON_TYPE_NUM:
			{
				long long number = 0;
				if(jsonValue.Get(number))
				{
					this->mSqlCommandStream << number;
				}
				return true;
			}
			case YYJSON_TYPE_BOOL:
			{
				bool value = false;
				if(jsonValue.Get(value))
				{
					this->mSqlCommandStream << value ? 1 : 0;
				}
				return true;
			}
			case YYJSON_TYPE_STR:
			{
				std::string value;
				if(jsonValue.Get(value))
				{
					this->mSqlCommandStream << "'";
					this->mSqlCommandStream.write(value.c_str(), value.size()) << "'";
				}
				return true;
			}
			case YYJSON_TYPE_OBJ:
			case YYJSON_TYPE_ARR:
			{
				this->mSqlCommandStream << "'";
				const std::string value = jsonValue.ToString();
				this->mSqlCommandStream.write(value.c_str(), value.size()) << "'";
				return true;
			}
		}
		return false;
	}

	bool SqlHelper::Delete(const std::string & table, const std::string & filter, int limit, std::string& sqlCommand)
	{
		json::r::Document document1;
        if(!document1.Decode(filter))
        {
            return false;
        }

		size_t index = 0;
		std::vector<const char *> keys;
		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "DELETE FROM " << table << " WHERE ";
		if(document1.GetKeys(keys) > 0)
		{
			std::unique_ptr<json::r::Value> value;
			for(const char * key : keys)
			{
				index++;
				if(document1.Get(key, value))
				{
					this->mSqlCommandStream << key << "=";
					if (!this->WriterToStream(this->mSqlCommandStream, *value))
					{
						return false;
					}
				}
				if (index + 1 < keys.size())
				{
					this->mSqlCommandStream << " AND ";
				}
			}
		}
		if(limit > 0)
		{
			this->mSqlCommandStream << " LIMIT " << limit;
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}


	bool SqlHelper::Insert(const std::string& table, json::r::Document& message, std::string& sqlCommand)
	{
		return this->ToSqlCommand(table, "INSERT", message, sqlCommand);
	}

	bool SqlHelper::Replace(const std::string& table, json::r::Document& message, std::string& sqlCommand)
	{
		return this->ToSqlCommand(table, "REPLACE", message, sqlCommand);
	}

	bool SqlHelper::Select(const google::protobuf::Message& message,
		const std::string& where, int limit, std::string& sqlCommand)
	{
		std::vector<std::string> fields;
		const pb::Descriptor * descriptor = message.GetDescriptor();
		for(int index = 0; index < descriptor->field_count(); index++)
		{
			const pb::FieldDescriptor * fieldDescriptor = descriptor->field(index);
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
		json::r::Document document;
		this->mSqlCommandStream.str("");
		if (!where.empty() && !document.Decode(where))
		{
			return false;
		}
		this->mSqlCommandStream << "SELECT ";
		if(fields.empty())
		{
			this->mSqlCommandStream << "*";
		}
		else
		{
			for (int index = 0; index < fields.size(); index++)
			{
				const std::string& field = fields.at(index);
				this->mSqlCommandStream << field;
				if (index != fields.size() - 1)
				{
					this->mSqlCommandStream << ",";
				}
			}
		}
		std::vector<const char *> keys;
		this->mSqlCommandStream << " FROM " << name;
		if(document.GetKeys(keys) <= 0)
		{
			sqlCommand = this->mSqlCommandStream.str();
			return true;
		}
		size_t index = 0;
		if(keys.size() > 0)
		{
			this->mSqlCommandStream << " WHERE ";
			std::unique_ptr<json::r::Value> value;
			for (const char* key: keys)
			{
				index++;
				if (document.Get(key, value))
				{
					this->mSqlCommandStream << key << "=";
					if (!this->WriterToStream(this->mSqlCommandStream, *value))
					{
						return false;
					}
					if (index + 1 < keys.size())
					{
						this->mSqlCommandStream << " AND ";
					}
				}
			}
		}
		if (limit != 0)
		{
			this->mSqlCommandStream << " LIMIT " << limit;
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}

	bool SqlHelper::Create(const std::string& tab,const pb::Message& message, const std::vector<std::string>& keys, std::string& sql)
	{
		const google::protobuf::Descriptor* descriptor = message.GetDescriptor();

		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "CREATE TABLE IF NOT EXISTS  " << tab << "(";
		for (int index = 0; index < descriptor->field_count(); index++)
		{
			const google::protobuf::FieldDescriptor* fieldDescriptor = descriptor->field(index);
			this->mSqlCommandStream << fieldDescriptor->name();
			switch (fieldDescriptor->type())
			{
				case pb::FieldDescriptor::Type::TYPE_INT32:
				case pb::FieldDescriptor::Type::TYPE_UINT32:
					this->mSqlCommandStream << " INT NOT NULL DEFAULT 0";
					break;
				case pb::FieldDescriptor::Type::TYPE_INT64:
				case pb::FieldDescriptor::Type::TYPE_UINT64:
					this->mSqlCommandStream << " BIGINT NOT NULL DEFAULT 0";
					break;
				case pb::FieldDescriptor::Type::TYPE_FLOAT:
				case pb::FieldDescriptor::Type::TYPE_DOUBLE:
					this->mSqlCommandStream << " DOUBLE NOT NULL DEFAULT 0";
					break;
				case pb::FieldDescriptor::Type::TYPE_STRING:
					this->mSqlCommandStream << " VARCHAR(64) NOT NULL DEFAULT ''";
					break;
				case pb::FieldDescriptor::Type::TYPE_BYTES:
					this->mSqlCommandStream << " BLOB(64) NOT NULL DEFAULT ''";
					break;
				case pb::FieldDescriptor::Type::TYPE_BOOL:
					this->mSqlCommandStream << " BOOLEAN NOT NULL DEFAULT 0";
					break;
				case pb::FieldDescriptor::TYPE_MESSAGE:
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
		this->mSqlCommandStream << ");";
		sql = this->mSqlCommandStream.str();
		return true;
	}

	bool SqlHelper::Create(const std::string& tab,const std::string & message, const std::vector<std::string>& keys, std::string& sql)
	{
		json::r::Document document;
		if(!document.Decode(message))
		{
			return false;
		}
		this->mSqlCommandStream.str("");
		std::vector<const char *> fields = document.GetAllKey();
		this->mSqlCommandStream << "CREATE TABLE IF NOT EXISTS " << tab  << "(" << '\n';
		for(size_t index = 0; index < fields.size(); index++)
		{
			const char * key = fields[index];
			std::unique_ptr<json::r::Value> jsonValue;
			if(!document.Get(key, jsonValue))
			{
				return false;
			}
			this->mSqlCommandStream << "  " << key;
			switch (jsonValue->GetType())
			{
				case YYJSON_SUBTYPE_REAL:
					this->mSqlCommandStream << " DOUBLE NOT NULL DEFAULT 0";
				case YYJSON_TYPE_NUM:
					this->mSqlCommandStream << " BIGINT NOT NULL DEFAULT 0";
					break;
				case YYJSON_TYPE_STR:
					this->mSqlCommandStream << " VARCHAR(64) NOT NULL DEFAULT ''";
					break;
				case YYJSON_TYPE_OBJ:
				case YYJSON_TYPE_ARR:
					this->mSqlCommandStream << " JSON";
					break;
				case YYJSON_TYPE_BOOL:
					this->mSqlCommandStream << " BOOLEAN NOT NULL DEFAULT 0";
					break;
				default:
					return false;
			}
			if(index != fields.size() - 1)
			{
				this->mSqlCommandStream << ",";
			}
			this->mSqlCommandStream << '\n';
		}

		if(!keys.empty())
		{
			this->mSqlCommandStream << ",PRIMARY KEY (";
			for (size_t index = 0; index < keys.size(); index++)
			{
				const std::string& key = keys.at(index);
				this->mSqlCommandStream << "`" << key << "`";
				if(index < keys.size() - 1)
				{
					this->mSqlCommandStream << ",";
				}
			}
			this->mSqlCommandStream << ")";
		}
		this->mSqlCommandStream << ")";
		sql = this->mSqlCommandStream.str();
		return true;
	}

	bool SqlHelper::CreateIndex(const std::string& table, const std::string& field, bool unique, std::string & sql)
	{
		if(table.empty() || field.empty())
		{
			return false;
		}
		this->mSqlCommandStream.str("");
		this->mSqlCommandStream << "CREATE ";
		if(unique)
		{
			this->mSqlCommandStream << "UNIQUE ";
		}
		this->mSqlCommandStream << "INDEX " << field << " ON " << table << "(" << field << ");";
		sql = this->mSqlCommandStream.str();
		return true;
	}
}