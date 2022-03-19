#include "MysqlComponent.h"
#include"Object/App.h"
#include"Pool/MessagePool.h"
#include"Coroutine/TaskComponent.h"
#include"DB/MysqlClient/TableOperator.h"
#include"Component/Scene/ThreadPoolComponent.h"

namespace Sentry
{
	bool MysqlComponent::Awake()
	{
		this->mMysqlPort = 0;
		this->mTaskManager = nullptr;
		this->mCorComponent = nullptr;
		const ServerConfig& config = App::Get().GetConfig();
		LOG_CHECK_RET_FALSE(config.GetValue("mysql", "ip", this->mMysqlIp));
		LOG_CHECK_RET_FALSE(config.GetValue("mysql", "port", this->mMysqlPort));
		LOG_CHECK_RET_FALSE(config.GetValue("mysql", "user", this->mDataBaseUser));
		LOG_CHECK_RET_FALSE(config.GetValue("mysql", "passwd", this->mDataBasePasswd));
		return true;
	}

	bool MysqlComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mTaskManager = this->GetComponent<ThreadPoolComponent>());
		return this->StartConnect() && this->InitMysqlTable();
	}

	MysqlClient* MysqlComponent::GetMysqlClient()
	{
		if (App::Get().IsMainThread())
		{
			LOG_ERROR("try in main thread invoke sql");
			return nullptr;
		}
		auto id = std::this_thread::get_id();
		auto iter = this->mMysqlSocketMap.find(id);
		return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
	}

	bool MysqlComponent::DropTable(const std::string& db)
	{
		const std::string sql = "drop database " + db;
		if (mysql_select_db(this->mMysqlSocket, db.c_str()) != 0)
		{
			return false;
		}
		if (mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.size()) != 0)
		{
			return false;
		}
		LOG_WARN("drop db : {0}", db);
		return true;
	}

	MysqlClient* MysqlComponent::ConnectMysql()
	{
		const char* ip = this->mMysqlIp.c_str();
		const unsigned short port = this->mMysqlPort;
		const char* passWd = this->mDataBasePasswd.c_str();
		const char* userName = this->mDataBaseUser.c_str();

		MysqlClient* mysqlSocket1 = mysql_init((MYSQL*)nullptr);
		this->mMysqlSocket = mysql_real_connect(mysqlSocket1, ip, userName, passWd, nullptr, port, nullptr,
			CLIENT_MULTI_STATEMENTS);
		if (this->mMysqlSocket == nullptr)
		{
			LOG_ERROR("connect mysql failure ", ip, port, userName, passWd);
			return nullptr;
		}
		LOG_INFO("connect mysql successful [{}:{}]", ip, port);
		return this->mMysqlSocket;
	}

	bool MysqlComponent::InitMysqlTable()
	{
		auto descriptorPool = google::protobuf::DescriptorPool::generated_pool();
		auto desc = descriptorPool->FindFileByName("db.proto");
		for (int x = 0; x < desc->message_type_count(); x++)
		{
			TableOperator tableOperator(this->mMysqlSocket);
			auto messageDesc = desc->message_type(x);
#ifdef __DEBUG__
			this->DropTable(messageDesc->name());
#endif
			LOG_CHECK_RET_FALSE(tableOperator.InitDb(messageDesc->name()));
			for (int y = 0; y < messageDesc->nested_type_count(); y++)
			{
				auto desc = messageDesc->nested_type(y);
				LOG_CHECK_RET_FALSE(tableOperator.InitTable(desc));
				const std::string table = messageDesc->name() + "." + desc->name();
				this->mSqlTableMap.emplace(table, desc->full_name());
				this->mSqlProtoMap.emplace(desc->full_name(), table);
			}
		}
		return true;
	}

	bool MysqlComponent::GetTableByProto(const Message& message, std::string& db)
	{
		const std::string& name = message.GetTypeName();
		auto iter = this->mSqlProtoMap.find(name);
		if (iter == this->mSqlProtoMap.end())
		{
			return false;
		}
		db = iter->second;
		return true;
	}

	bool MysqlComponent::GetProtoByTable(const std::string& tab, std::string& proto)
	{
		auto iter = this->mSqlTableMap.find(tab);
		if (iter == this->mSqlTableMap.end())
		{
			return false;
		}
		proto = iter->second;
		return true;
	}

	bool MysqlComponent::StartConnect()
	{
		const std::vector<TaskThread*>& threadTasks = this->mTaskManager->GetThreads();
		LOG_CHECK_RET_FALSE(threadTasks.size() > 0);
		for (TaskThread* taskThread : threadTasks)
		{
			auto mysqlSocket = this->ConnectMysql();
			if (mysqlSocket == nullptr)
			{
				return false;
			}
			this->mMysqlSocketMap.insert(std::make_pair(taskThread->GetThreadId(), mysqlSocket));
		}
		return true;
	}

	bool MysqlComponent::ToSqlCommand(const std::string& table, const std::string& cmd,
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

	bool MysqlComponent::ToSqlCommand(const s2s::Mysql::Update& messageData, std::string& sqlCommand)
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

	bool MysqlComponent::WriterToStream(std::stringstream& stream, const rapidjson::Value& jsonValue)
	{
		if (jsonValue.IsString())
		{
			const char* str = jsonValue.GetString();
			size_t size = jsonValue.GetStringLength();
			this->mSqlCommandStream << "'" << std::string(str, size) << "'";
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

	bool MysqlComponent::ToSqlCommand(const s2s::Mysql::Delete& messageData, std::string& sqlCommand)
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

	bool MysqlComponent::ToSqlCommand(const s2s::Mysql::Add& request, std::string& sqlCommand)
	{
		std::shared_ptr<Message> message = Helper::Proto::NewByData(request.data());
		if (message == nullptr)
		{
			return false;
		}
		const std::string& table = request.table();
		return this->ToSqlCommand(table, "insert", *message, sqlCommand);
	}

	bool MysqlComponent::ToSqlCommand(const s2s::Mysql::Save& request, std::string& sqlCommand)
	{
		std::shared_ptr<Message> message = Helper::Proto::NewByData(request.data());
		if (message == nullptr)
		{
			return false;
		}
		const std::string& table = request.table();
		return this->ToSqlCommand(table, "replace", *message, sqlCommand);
	}

	bool MysqlComponent::ToSqlCommand(const s2s::Mysql::Query& request, std::string& sqlCommand)
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
			this->mSqlCommandStream << key << "=";
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