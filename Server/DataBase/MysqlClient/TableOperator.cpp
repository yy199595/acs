#include"TableOperator.h"
#include<fstream>
#include<Manager/MysqlManager.h>
namespace SoEasy
{
	TableOperator::TableOperator(SayNoMysqlSocket * socket, const std::string db, const std::string & path)
		:mConfigPath(path)
	{
		this->mDataBase = db;
		this->mMysqlSocket = socket;
	}

	bool TableOperator::InitMysqlTable()
	{
		std::fstream fs(this->mConfigPath, std::ios::in);
		if (!fs.is_open())
		{
			SayNoDebugError("not find file " << this->mConfigPath);
			return false;
		}
		std::string json;
		std::string temp;
		while (std::getline(fs, temp))
		{
			json.append(temp);
		}
		this->mDocument.Parse(json.c_str(), json.size());
		if (this->mDocument.HasParseError())
		{
			SayNoDebugError("parse " << this->mConfigPath << " json fail");
			return false;
		}

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
			const std::string key = iter->value.FindMember("primatykey")->value.GetString();
			std::string sql = "desc " + table;
			if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
			{
				if (!this->CreateMysqlTable(table, name, key))
				{
					SayNoDebugError("create new table " << table << " fail " << mysql_error(this->mMysqlSocket));
					return false;
				}
				SayNoDebugInfo("create new table success " << table);
			}
			else
			{
				this->UpdateMysqlTable(table, name, key);
			}
		}
		return true;
	}

	bool TableOperator::UpdateMysqlTable(const std::string table, const std::string name, const std::string key)
	{
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor->FindFieldByName(key) == nullptr)
		{
			SayNoDebugError("create " << name << " error 'key' not find");
			return false;
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
			sqlStream << " int(20) NOT NULL DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
		{
			sqlStream << " bigint(32) NOT NULL DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_FLOAT)
		{
			sqlStream << " float(20) NOT NULL DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_DOUBLE)
		{
			sqlStream << " double(32) NOT NULL DEFAULT 0";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_STRING)
		{
			sqlStream << " varchar(64) NOT NULL DEFAULT ''";
		}
		else if (fieldDesc->type() == FieldDescriptor::TYPE_BYTES)
		{
			sqlStream << " BLOB(64) NOT NULL DEFAULT ''";
		}
		const std::string sql = sqlStream.str();
		if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.length()) != 0)
		{
			SayNoDebugError(mysql_error(mMysqlSocket));
			return false;
		}
		return true;
	}

	bool TableOperator::CreateMysqlTable(const std::string table, const std::string name, const std::string key)
	{
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor->FindFieldByName(key) == nullptr)
		{
			SayNoDebugError("create " << name << " error 'key' not find");
			return false;
		}
		std::stringstream sqlCommand;
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
				sqlCommand << "int(20) NOT NULL DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_INT64)
			{
				sqlCommand << "bigint(32) NOT NULL DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_FLOAT)
			{
				sqlCommand << "float(20) NOT NULL DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_DOUBLE)
			{
				sqlCommand << "double(32) NOT NULL DEFAULT 0,\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_STRING)
			{
				sqlCommand << "varchar(64) NOT NULL DEFAULT '',\n";
			}
			else if (fileDesc->type() == FieldDescriptor::TYPE_BYTES)
			{
				sqlCommand << "BLOB(64) NOT NULL DEFAULT '',\n";
			}
			else
			{
				assert(false);
			}
		}
		sqlCommand << "PRIMARY KEY (`" << key << "`)\n";
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