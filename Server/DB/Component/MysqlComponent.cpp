#include "MysqlComponent.h"
#include"Core/App.h"
#include"Coroutine/TaskComponent.h"
#include"MysqlClient/TableOperator.h"
#include"Component/Scene/ThreadPoolComponent.h"

namespace GameKeeper
{
    bool MysqlComponent::Awake()
    {
        this->mMysqlPort = 0;
        this->mTaskManager = nullptr;
        this->mCorComponent = nullptr;
		const ServerConfig & config = App::Get().GetConfig();
		this->mSqlPath = App::Get().GetServerPath().GetConfigPath() + "sql.json";

        LOG_CHECK_RET_FALSE(config.GetValue("Mysql", "ip", this->mMysqlIp));
        LOG_CHECK_RET_FALSE(config.GetValue("Mysql", "port", this->mMysqlPort));
        LOG_CHECK_RET_FALSE(config.GetValue("Mysql", "user", this->mDataBaseUser));
		LOG_CHECK_RET_FALSE(config.GetValue("Mysql", "passwd", this->mDataBasePasswd));
        return true;
    }

    bool MysqlComponent::LateAwake()
    {
        LOG_CHECK_RET_FALSE(this->mTaskManager = this->GetComponent<ThreadPoolComponent>());
        LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<TaskComponent>());
        return this->StartConnect() && this->InitMysqlTable();
    }

    GKMysqlSocket *MysqlComponent::GetMysqlSocket()
    {
        if(App::Get().IsMainThread())
        {
            LOG_ERROR("try in main thread invoke sql");
            return nullptr;
        }
		auto id = std::this_thread::get_id();
        auto iter = this->mMysqlSocketMap.find(id);
        return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
    }

    bool MysqlComponent::DropTable(const std::string & db)
    {
        const std::string sql = "drop database " + db;
        if (mysql_select_db(this->mMysqlSockt, db.c_str()) != 0)
        {
            return false;
        }
        if (mysql_real_query(this->mMysqlSockt, sql.c_str(), sql.size()) != 0)
        {
            return false;
        }
        LOG_WARN("drop db : " << db);
        return true;
    }

	GKMysqlSocket * MysqlComponent::ConnectMysql()
	{
		const char *ip = this->mMysqlIp.c_str();
		const unsigned short port = this->mMysqlPort;
		const char *passWd = this->mDataBasePasswd.c_str();
		const char *userName = this->mDataBaseUser.c_str();

		GKMysqlSocket *mysqlSocket1 = mysql_init((MYSQL *)nullptr);
		this->mMysqlSockt  = mysql_real_connect(mysqlSocket1, ip, userName, passWd, nullptr, port, nullptr,
			CLIENT_MULTI_STATEMENTS);
		if (this->mMysqlSockt == nullptr)
		{
			LOG_ERROR("connect mysql failure "
				<< ip << ":" << port << "  " << userName << " " << passWd);
			return nullptr;
		}
		LOG_INFO("connect mysql successful [" << ip << ":" << port << "]");
		return this->mMysqlSockt;
	}

    bool MysqlComponent::InitMysqlTable()
    {
        auto descriptorPool = google::protobuf::DescriptorPool::generated_pool();
        auto desc = descriptorPool->FindFileByName("db.proto");
        for (int x = 0; x < desc->message_type_count(); x++)
        {
            TableOperator tableOperator(this->mMysqlSockt);
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
                this->mSqlTableMap.emplace(table,  desc->full_name());
                this->mSqlProtoMap.emplace(desc->full_name(), table);
            }
        }
        return true;
    }

    bool MysqlComponent::GetTableByProto(const Message &message, std::string & db)
    {
        const std::string & name = message.GetTypeName();
        auto iter = this->mSqlProtoMap.find(name);
        if(iter == this->mSqlProtoMap.end())
        {
            return false;
        }
        db = iter->second;
        return true;
    }

    bool MysqlComponent::GetProtoByTable(const std::string &tab, std::string &proto)
    {
        auto iter = this->mSqlTableMap.find(tab);
        if(iter == this->mSqlTableMap.end())
        {
            return false;
        }
        proto = iter->second;
        return true;
    }


    bool MysqlComponent::StartConnect()
    {
		const std::vector<TaskThread *> & threadTasks = this->mTaskManager->GetThreads();
		for (TaskThread * taskThread : threadTasks)
		{
			auto mysqlSocket = this->ConnectMysql();
			if(mysqlSocket == nullptr)
            {
                return false;
            }
			this->mMysqlSocketMap.insert(std::make_pair(taskThread->GetThreadId(), mysqlSocket));
		}
        return true;
    }

    bool MysqlComponent::GetAddSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string table;
        if(!this->GetTableByProto(messageData, table))
        {
            return false;
        }
        this->mSqlCommandStream.str("");
        std::vector<const FieldDescriptor *> fieldList;
        const Reflection *pReflection = messageData.GetReflection();
        pReflection->ListFields(messageData, &fieldList);
        mSqlCommandStream << "insert into " << table << "(";
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

        for (auto fieldDesc: fieldList)
        {
            switch (fieldDesc->type())
            {
                case FieldDescriptor::Type::TYPE_STRING:
                case FieldDescriptor::Type::TYPE_BYTES:
                    mSqlCommandStream << "'" << pReflection->GetString(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_INT32:
                    mSqlCommandStream << "'" << pReflection->GetInt32(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_UINT32:
                    mSqlCommandStream << "'" << pReflection->GetUInt32(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_INT64:
                    mSqlCommandStream << "'" << pReflection->GetInt64(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_UINT64:
                    mSqlCommandStream << "'" << pReflection->GetUInt64(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_FLOAT:
                    mSqlCommandStream << "'" << pReflection->GetFloat(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_DOUBLE:
                    mSqlCommandStream << "'" << pReflection->GetDouble(messageData, fieldDesc) << "',";
                    break;
                default:
                    return false;
            }
        }
        sqlCommand = mSqlCommandStream.str();
        sqlCommand[sqlCommand.size() - 1] = ')';
        return true;
    }

    bool MysqlComponent::GetSaveSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string table;
        if(!this->GetTableByProto(messageData, table))
        {
            return false;
        }
        this->mSqlCommandStream.str("");
        this->mSqlCommandStream2.str("");
        const Reflection *pReflection = messageData.GetReflection();
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        mSqlCommandStream << "update " << table << " set ";
        const FieldDescriptor * firstFieldDesc = pDescriptor->FindFieldByNumber(1);
        mSqlCommandStream2 << " where " << firstFieldDesc->name() << "=";

        if (firstFieldDesc->type() == FieldDescriptor::TYPE_STRING)
        {
            std::string value = pReflection->GetString(messageData, firstFieldDesc);
            if (value == firstFieldDesc->default_value_string())
            {
                return false;
            }
            mSqlCommandStream2 << "'" << value << "'";
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_INT64)
        {
            long long value = pReflection->GetInt64(messageData, firstFieldDesc);
            if (value == firstFieldDesc->default_value_int64())
            {
                return false;
            }
            mSqlCommandStream2 << value;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_UINT64)
        {
            unsigned long long value = pReflection->GetUInt64(messageData, firstFieldDesc);
            if (value == firstFieldDesc->default_value_uint64())
            {
                return false;
            }
            mSqlCommandStream2 << value;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_INT32)
        {
            int value = pReflection->GetInt32(messageData, firstFieldDesc);
            if (value == firstFieldDesc->default_value_int32())
            {
                return false;
            }
            mSqlCommandStream2 << value;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_UINT32)
        {
            unsigned int value = pReflection->GetUInt32(messageData, firstFieldDesc);
            if (value == firstFieldDesc->default_value_uint32())
            {
                return false;
            }
            mSqlCommandStream2 << value;
        }
        else
        {
            return false;
        }

        for(int index = 2; index < pDescriptor->field_count(); index++)
        {
            auto fieldDesc = pDescriptor->FindFieldByNumber(index);
            mSqlCommandStream << fieldDesc->name() << "=";
            switch (fieldDesc->type())
            {
                case FieldDescriptor::Type::TYPE_BYTES:
                case FieldDescriptor::Type::TYPE_STRING:
                    mSqlCommandStream << "'" << pReflection->GetString(messageData, fieldDesc) << "',";
                    break;
                case FieldDescriptor::Type::TYPE_INT64:
                    mSqlCommandStream << pReflection->GetInt64(messageData, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_UINT64:
                    mSqlCommandStream << pReflection->GetUInt64(messageData, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_INT32:
                    mSqlCommandStream << pReflection->GetInt32(messageData, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_UINT32:
                    mSqlCommandStream << pReflection->GetUInt32(messageData, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_FLOAT:
                    mSqlCommandStream << pReflection->GetFloat(messageData, fieldDesc) << ",";
                    break;
                case FieldDescriptor::Type::TYPE_DOUBLE:
                    mSqlCommandStream << pReflection->GetDouble(messageData, fieldDesc) << ",";
                    break;
                default:
                    return false;
            }
        }
        sqlCommand = mSqlCommandStream.str();
        sqlCommand[sqlCommand.size() - 1] = ' ';
        sqlCommand.append(mSqlCommandStream2.str());

        return true;
    }

    bool MysqlComponent::GetQuerySqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string table;
        if (!this->GetTableByProto(messageData, table))
        {
            return false;
        }
        mSqlCommandStream.str("");
        this->mSqlCommandStream << "select ";
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();
        const FieldDescriptor *firstFieldDesc = pDescriptor->FindFieldByNumber(1);
        if (firstFieldDesc == nullptr)
        {
            return false;
        }

        for (int index = 0; index < pDescriptor->field_count(); index++)
        {
            auto typeDesc = pDescriptor->field(index);
            if (index < pDescriptor->field_count() - 1)
            {
                mSqlCommandStream << typeDesc->name() << ",";
                continue;
            }
            mSqlCommandStream << typeDesc->name();
        }
        this->mSqlCommandStream << " from " << table << " where ";

        const std::string &key = firstFieldDesc->name();
        if (firstFieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
        {
            const std::string val = pReflection->GetString(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(val != firstFieldDesc->default_value_string());
            mSqlCommandStream << key << "='" << val << "'";
        }
        else if (firstFieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
        {
            long long val = pReflection->GetInt64(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(val != firstFieldDesc->default_value_int64());
            mSqlCommandStream << key << "=" << val;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
        {
            unsigned long long val = pReflection->GetUInt64(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(val != firstFieldDesc->default_value_uint64());
            mSqlCommandStream << key << "=" << val;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_INT32)
        {
            int val = pReflection->GetInt32(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(val != firstFieldDesc->default_value_int32());
            mSqlCommandStream << key << "=" << val;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_UINT32)
        {
            unsigned int val = pReflection->GetUInt32(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(val != firstFieldDesc->default_value_uint32());
            mSqlCommandStream << key << "=" << val;
        }
        sqlCommand = mSqlCommandStream.str();
        return true;
    }

    bool MysqlComponent::GetDeleteSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
         std::string table;
        if(!this->GetTableByProto(messageData, table))
        {
            return false;
        }
        this->mSqlCommandStream.str("");
        const Reflection *pReflection = messageData.GetReflection();
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const FieldDescriptor *firstFieldDesc = pDescriptor->FindFieldByNumber(1);
        mSqlCommandStream << "delete from " << table << " where " << firstFieldDesc->name() << "=";

        if (firstFieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
        {
            const std::string key = pReflection->GetString(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(key != firstFieldDesc->default_value_string());
            mSqlCommandStream << "'" << key << "'";
        }
        else if (firstFieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
        {
            int key = pReflection->GetInt32(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(key != firstFieldDesc->default_value_int32())
            mSqlCommandStream << key;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_UINT32)
        {
            unsigned int key = pReflection->GetUInt32(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(key != firstFieldDesc->default_value_uint32())
            mSqlCommandStream << key;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
        {
            long long key = pReflection->GetInt64(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(key != firstFieldDesc->default_value_int64())
            mSqlCommandStream << key;
        }
        else if (firstFieldDesc->type() == FieldDescriptor::TYPE_UINT64)
        {
            unsigned long long key = pReflection->GetUInt64(messageData, firstFieldDesc);
            LOG_CHECK_RET_FALSE(key != firstFieldDesc->default_value_uint64())
            mSqlCommandStream << key;
        }
        else
        {
            return false;
        }
        sqlCommand = mSqlCommandStream.str();
        return true;
    }
}