#include"TableOperator.h"
#include<fstream>
#include<Protocol/db.pb.h>
#include<Manager/MysqlManager.h>
namespace SoEasy
{
	TableOperator::TableOperator(SayNoMysqlSocket * socket, const std::string db, rapidjson::Document & doc)
		:mDocument(doc)
	{
		this->mDataBase = db;
		this->mMysqlSocket = socket;
	}

	bool TableOperator::InitMysqlTable()
	{
		if (mysql_select_db(this->mMysqlSocket, this->mDataBase.c_str()) != 0)
		{
			std::string sql = "Create DataBase " + this->mDataBase;
			if (mysql_real_query(mMysqlSocket, sql.c_str(), sql.length()) != 0)
			{
				SayNoDebugError("create " << this->mDataBase << " db fail");
				return false;
			}
			SayNoDebugLog("create " << this->mDataBase << " db successful");
			mysql_select_db(this->mMysqlSocket, this->mDataBase.c_str());
		}

		for (auto iter = this->mDocument.MemberBegin(); iter != this->mDocument.MemberEnd(); iter++)
		{
			if (!iter->name.IsString() || !iter->value.IsObject())
			{
				SayNoDebugError("json config error");
				return false;
			}
			const std::string table = iter->name.GetString();
			const std::string name = iter->value.FindMember("protobuf")->value.GetString();
			auto iter1 = iter->value.FindMember("keys");
			if (iter1 == iter->value.MemberEnd() || !iter1->value.IsArray())
			{
				SayNoDebugError(table << " sql table config error");
				return false;
			}
			std::vector<std::string> keys;
			for (unsigned int index = 0; index < iter1->value.Size(); index++)
			{
				const std::string key = iter1->value[index].GetString();
				keys.push_back(key);
			}
			std::string sql = "desc " + table;
			if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
			{
				if (!this->CreateMysqlTable(table, name, keys))
				{
					SayNoDebugError("create new table " << table << " fail " << mysql_error(this->mMysqlSocket));
					return false;
				}
				SayNoDebugInfo("create new table success " << table);
			}
			else
			{
				this->UpdateMysqlTable(table, name, keys);
			}
		}
		return true;
	}

	bool TableOperator::UpdateMysqlTable(const std::string table, const std::string name, const std::vector<std::string> & keys)
	{
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		for (const std::string & key : keys)
		{
			if (pDescriptor->FindFieldByName(key) == nullptr)
			{
				SayNoDebugError("create " << name << " error 'key' not find");
				return false;
			}
		}
		MysqlQueryResult * queryResult = mysql_store_result(this->mMysqlSocket);
		if (queryResult == nullptr)
		{
			return false;
		}
		std::set<std::string> fieldSet;
		unsigned long rowCount = mysql_num_rows(queryResult);
		unsigned int fieldCount = mysql_field_count(mMysqlSocket);
		for (unsigned long count = 0; count < rowCount; count++)
		{
			MYSQL_ROW row = mysql_fetch_row(queryResult);
			unsigned long * lengths = mysql_fetch_lengths(queryResult);

			for (size_t index = 0; index < fieldCount; index++)
			{
				fieldSet.insert(std::string(row[index], (int)lengths[index]));
			}
		}

		for (int index = 1; index <= pDescriptor->field_count(); index++)
		{
			const FieldDescriptor * fileDesc = pDescriptor->FindFieldByNumber(index);
			if (fileDesc == nullptr)
			{
				continue;
			}
			auto iter = fieldSet.find(fileDesc->name());
			if (iter == fieldSet.end())
			{
				if (!this->AddNewField(table, fileDesc))
				{
					SayNoDebugError("[mysql error ] " << mysql_error(this->mMysqlSocket));
					SayNoDebugError("add field " << fileDesc->name() << " to " << table << " fail");
					return false;
				}
				SayNoDebugLog("add field " << fileDesc->name() << " to " << table << " successful");
			}		
		}
		return true;
	}

	bool TableOperator::AddNewField(const std::string table, const FieldDescriptor * fieldDesc)
	{
		std::stringstream sqlStream;
		sqlStream << "alter table " << table << " add " << fieldDesc->name();
		if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
		{
			sqlStream << " int(20) DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
		{
			sqlStream << " bigint(32) DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_FLOAT)
		{
			sqlStream << " float(20) DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_DOUBLE)
		{
			sqlStream << " double(32) DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_STRING)
		{
			sqlStream << " varchar(64) DEFAULT NULL";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_BYTES)
		{
			sqlStream << " BLOB(64) DEFAULT NULL";
		}
		const std::string sql = sqlStream.str();
		if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
		{
			SayNoDebugError(mysql_error(mMysqlSocket));
			return false;
		}
		return true;
	}

	bool TableOperator::CreateMysqlTable(const std::string table, const std::string name, const std::vector<std::string> & keys)
	{
		db::UserAccountData account;
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		for (const std::string & key : keys)
		{
			if (pDescriptor->FindFieldByName(key) == nullptr)
			{
				SayNoDebugError("create " << name << " error 'key' not find");
				return false;
			}
		}	
		std::stringstream sqlCommand;
		auto IsHasField = [&keys](const std::string & name)->bool
		{
			for (const std::string & key : keys)
			{
				if (key == name)
				{
					return true;
				}
			}
			return false;
		};
		sqlCommand << "create table `" << table << "`(\n";
		for (int index = 1; index <= pDescriptor->field_count(); index++)
		{
			const FieldDescriptor * fileDesc = pDescriptor->FindFieldByNumber(index);
			if (fileDesc == nullptr)
			{
				continue;
			}
			sqlCommand << "`" << fileDesc->name() << "` ";
			if (fileDesc->type() == FieldDescriptor::TYPE_INT32)
			{
				if (IsHasField(fileDesc->name()))
				{
					sqlCommand << "int(20) NOT NULL,\n";
					continue;
				}
				sqlCommand << "int(20) DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_INT64)
			{
				if (IsHasField(fileDesc->name()))
				{
					sqlCommand << "bigint(32) NOT NULL,\n";
					continue;
				}
				sqlCommand << "bigint(32) DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_FLOAT)
			{
				SayNoAssertRetFalse_F(IsHasField(fileDesc->name()));
				sqlCommand << "float(20) NOT NULL DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_DOUBLE)
			{
				SayNoAssertRetFalse_F(IsHasField(fileDesc->name()));
				sqlCommand << "double(32) DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_STRING)
			{
				if (IsHasField(fileDesc->name()))
				{
					sqlCommand << "varchar(20) NOT NULL,\n";
					continue;
				}
				sqlCommand << "varchar(64) DEFAULT NULL,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_BYTES)
			{
				SayNoAssertRetFalse_F(IsHasField(fileDesc->name()));
				sqlCommand << "BLOB(64) DEFAULT NULL,\n";
			}
			else
			{
				assert(false);
			}
		}
		sqlCommand << "PRIMARY KEY (";
		for (size_t index = 0; index < keys.size(); index++)
		{
			const std::string & key = keys[index];
			if (index == keys.size() - 1)
			{			
				sqlCommand << "`" << key << "`)\n";
				break;
			}
			sqlCommand << "`" << key << "`,";
		}
		sqlCommand << ")ENGINE=InnoDB DEFAULT CHARSET = utf8;";
		const std::string sql = sqlCommand.str();
		if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
		{
			SayNoDebugError(mysql_error(mMysqlSocket));
			return false;
		}
		return true;

	}
}