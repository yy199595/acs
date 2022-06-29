#include "MysqlHelper.h"
#include"App/App.h"
#include"Component/Scene/MessageComponent.h"
namespace Sentry
{
	bool MysqlHelper::ToSqlCommand(const std::string& table, const std::string& cmd,
		Message& message, std::string& sql)
	{
		this->mSqlCommandStream.str("");
		std::vector<const FieldDescriptor*> fieldList;
		const Reflection* pReflection = message.GetReflection();
		pReflection->ListFields(message, &fieldList);
		mSqlCommandStream << cmd << " into " << table << "(";
		for (size_t index = 0; index < fieldList.size(); index++)
		{
			if (index < fieldList.size() - 1)
			{
				mSqlCommandStream << fieldList[index]->name() << ",";
				continue;
			}
			mSqlCommandStream << fieldList[index]->name();
		}
		mSqlCommandStream << ")values(";

		for (auto fieldDesc : fieldList)
		{
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
			default:
				return false;
			}
		}
		sql = mSqlCommandStream.str();
		sql[sql.size() - 1] = ')';
		return true;
	}

	bool MysqlHelper::ToSqlCommand(const s2s::Mysql::Update& messageData, std::string& sqlCommand)
	{
		rapidjson::Document whereJsonDocument;
		rapidjson::Document updateJsonDocument;
		const std::string& whereJson = messageData.where_json();
		const std::string& updateJson = messageData.update_json();
		if (whereJsonDocument.Parse(whereJson.c_str(), whereJson.size()).HasParseError())
		{
			return false;
		}
		if (updateJsonDocument.Parse(updateJson.c_str(), updateJson.size()).HasParseError())
		{
			return false;
		}
		size_t index = 0;
		this->mSqlCommandStream.str("");
		const std::string& table = messageData.table();
		this->mSqlCommandStream << "update " << table << " set ";
		auto iter1 = updateJsonDocument.MemberBegin();
		for (; iter1 != updateJsonDocument.MemberEnd(); iter1++, index++)
		{
			const rapidjson::Value& jsonValue = iter1->value;
			this->mSqlCommandStream << iter1->name.GetString() << "=";
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < updateJsonDocument.MemberCount())
			{
				this->mSqlCommandStream << ",";
			}
		}
		index = 0;
		this->mSqlCommandStream << " where ";
		auto iter2 = whereJsonDocument.MemberBegin();
		for (; iter2 != whereJsonDocument.MemberEnd(); iter2++, index++)
		{
			const rapidjson::Value& jsonValue = iter2->value;
			this->mSqlCommandStream << iter2->name.GetString() << "=";
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < whereJsonDocument.MemberCount())
			{
				this->mSqlCommandStream << " and ";
			}
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
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
		return false;
	}

	bool MysqlHelper::ToSqlCommand(const s2s::Mysql::Delete& messageData, std::string& sqlCommand)
	{
		rapidjson::Document jsonDocument;
		const std::string& json = messageData.where_json();
		if (jsonDocument.Parse(json.c_str(), json.size()).HasParseError())
		{
			return false;
		}
		size_t index = 0;
		this->mSqlCommandStream.str("");
		const std::string& table = messageData.table();
		this->mSqlCommandStream << "delete from " << table << " where ";
		auto iter = jsonDocument.MemberBegin();
		for (; iter != jsonDocument.MemberEnd(); iter++, index++)
		{
			const char* key = iter->name.GetString();
			this->mSqlCommandStream << key << "=";
			rapidjson::Value& jsonValue = iter->value;
			if (!this->WriterToStream(this->mSqlCommandStream, jsonValue))
			{
				return false;
			}
			if (index + 1 < jsonDocument.MemberCount())
			{
				this->mSqlCommandStream << " and ";
			}
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}

	bool MysqlHelper::ToSqlCommand(const s2s::Mysql::Add& request, std::string& sqlCommand)
	{
		MessageComponent * messageComponent = App::Get()->GetComponent<MessageComponent>();
		std::shared_ptr<Message> message = messageComponent->New(request.data());
		if (message == nullptr)
		{
			return false;
		}
		const std::string& table = request.table();
		return this->ToSqlCommand(table, "insert", *message, sqlCommand);
	}

	bool MysqlHelper::ToSqlCommand(const s2s::Mysql::Save& request, std::string& sqlCommand)
	{
		MessageComponent * messageComponent = App::Get()->GetComponent<MessageComponent>();
		std::shared_ptr<Message> message = messageComponent->New(request.data());
		if (message == nullptr)
		{
			return false;
		}
		const std::string& table = request.table();
		return this->ToSqlCommand(table, "replace", *message, sqlCommand);
	}

	bool MysqlHelper::ToSqlCommand(const s2s::Mysql::Query& request, std::string& sqlCommand)
	{
		this->mSqlCommandStream.str("");
		rapidjson::Document jsonDocument;
		const std::string& json = request.where_json();
		if (jsonDocument.Parse(json.c_str(), json.size()).HasParseError())
		{
			return false;
		}
		if (!jsonDocument.IsObject())
		{
			return false;
		}
		this->mSqlCommandStream << "select * from " << request.table();
		if (jsonDocument.MemberCount() == 0)
		{
			sqlCommand = this->mSqlCommandStream.str();
			return true;
		}
		size_t index = 0;
		this->mSqlCommandStream << " where ";
		auto iter = jsonDocument.MemberBegin();
		for (; iter != jsonDocument.MemberEnd(); iter++, index++)
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
			if (index + 1 < jsonDocument.MemberCount())
			{
				this->mSqlCommandStream << " and ";
			}
		}
		sqlCommand = this->mSqlCommandStream.str();
		return true;
	}
}