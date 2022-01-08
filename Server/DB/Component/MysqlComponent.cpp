#include "MysqlComponent.h"
#include <fstream>
#include <utility>
#include "Core/App.h"
#include "Component/Scene/ThreadPoolComponent.h"
#include "MysqlClient/TableOperator.h"
#include "Coroutine/TaskComponent.h"

namespace GameKeeper
{
    SqlTableConfig::SqlTableConfig(const std::string db, std::string  tab, const std::string pb)
            : mDb(db), mTableName(std::move(tab)), mProtobufName(pb)
    {
    }

    void SqlTableConfig::AddKey(const std::string& key)
    {
        this->mKeys.insert(key);
    }

    bool SqlTableConfig::HasKey(const std::string &key) const
    {
        auto iter = this->mKeys.find(key);
        return iter != this->mKeys.end();
    }
}

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

    bool MysqlComponent::DropTable(const std::string & db,const std::string &name)
    {
        const std::string sql = "drop table " + name;
        if (mysql_select_db(this->mMysqlSockt, db.c_str()) != 0)
        {
            return false;
        }
        if (mysql_real_query(this->mMysqlSockt, sql.c_str(), sql.size()) != 0)
        {
            return false;
        }
        LOG_WARN("drop table : " << name);
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

    const SqlTableConfig *MysqlComponent::GetConfigByTab(const std::string &tab) const
    {
        auto iter = this->mSqlConfigMap.find(tab);
        return iter != this->mSqlConfigMap.end() ? iter->second : nullptr;
    }

    const SqlTableConfig *MysqlComponent::GetCondifByProto(const std::string &proro) const
    {
        auto iter = this->mTablePbMap.find(proro);
        return iter != this->mTablePbMap.end() ? iter->second : nullptr;
    }

    bool MysqlComponent::InitMysqlTable()
    {
        std::fstream fs(this->mSqlPath, std::ios::in);
        if (!fs.is_open())
        {
            LOG_ERROR("not find file " << this->mSqlPath);
            return false;
        }
        std::string json;
        std::string temp;
        while (std::getline(fs, temp))
        {
            json.append(temp);
        }
        rapidjson::Document document;
        document.Parse(json.c_str(), json.size());
        if (document.HasParseError())
        {
            LOG_ERROR("parse " << mSqlPath << " json fail");
            return false;
        }
        for (auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
        {
            if (!iter->name.IsString() || !iter->value.IsObject())
            {
                LOG_ERROR(mSqlPath << " error");
                return false;
            }
            rapidjson::Value  & jsonObject = iter->value;
            const std::string db(iter->name.GetString());

            for(auto iter1 = jsonObject.MemberBegin(); iter1 != jsonObject.MemberEnd(); iter1++)
            {
                const std::string tab(iter1->name.GetString());
                const std::string proto(iter1->value["protobuf"].GetString());

                SqlTableConfig * tableConfig = new SqlTableConfig(db, db + "." + tab, proto);
                rapidjson::Value::Array jsonArray = iter1->value["keys"].GetArray();

                for (int index = 0; index < jsonArray.Size(); index++)
                {
                    tableConfig->AddKey(jsonArray[index].GetString());
                }
                this->mTablePbMap.emplace(proto, tableConfig);
                this->mSqlConfigMap.emplace(tableConfig->mTableName, tableConfig);
#ifdef __DEBUG__
                this->DropTable(db, tab);
#endif
            }
            TableOperator tableOper(this->mMysqlSockt, db, jsonObject);
            if(!tableOper.InitMysqlTable())
            {
                return false;
            }
        }
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

    bool MysqlComponent::GetAddSqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand)
    {
        const std::string typeName = messageData.GetTypeName();
        const SqlTableConfig * sqlTableConfig = this->GetCondifByProto(typeName);
        if(sqlTableConfig == nullptr)
        {
            LOG_ERROR("get sql config by " << typeName << " failure");
            return false;
        }

        db = sqlTableConfig->mDb;
        this->mSqlCommandStream.str("");
        std::vector<const FieldDescriptor *> fieldList;
        const std::string & tableName = sqlTableConfig->mTableName;

        const Reflection *pReflection = messageData.GetReflection();
        pReflection->ListFields(messageData, &fieldList);

        mSqlCommandStream << "insert into " << sqlTableConfig->mTableName << "(";
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
            if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
            {
                mSqlCommandStream << "'" << pReflection->GetString(messageData, fieldDesc) << "',";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_BYTES)
            {
                mSqlCommandStream << "'" << pReflection->GetString(messageData, fieldDesc) << "',";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
            {
                mSqlCommandStream << pReflection->GetInt64(messageData, fieldDesc) << ",";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
            {
                mSqlCommandStream << pReflection->GetUInt64(messageData, fieldDesc) << ",";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
            {
                mSqlCommandStream << pReflection->GetInt32(messageData, fieldDesc) << ",";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT32)
            {
                mSqlCommandStream << pReflection->GetUInt32(messageData, fieldDesc) << ",";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_FLOAT)
            {
                mSqlCommandStream << pReflection->GetFloat(messageData, fieldDesc) << ",";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_DOUBLE)
            {
                mSqlCommandStream << pReflection->GetDouble(messageData, fieldDesc) << ",";
            }
            else
            {
                return false;
            }
        }
        sqlCommand = mSqlCommandStream.str();
        sqlCommand[sqlCommand.size() - 1] = ')';
        return true;

    }

    bool MysqlComponent::GetSaveSqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand)
    {
        const std::string typeName = messageData.GetTypeName();
        const SqlTableConfig * sqlTableConfig = this->GetCondifByProto(typeName);
        if(sqlTableConfig == nullptr)
        {
            LOG_ERROR("get sql config by " << typeName << " failure");
            return false;
        }

        db = sqlTableConfig->mDb;
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();


        std::vector<const FieldDescriptor *> fieldList;
        pReflection->ListFields(messageData, &fieldList);

        mSqlCommandStream2 << " where ";
        mSqlCommandStream << "update " << sqlTableConfig->mTableName << " set ";

        int index =0;
        for(const std::string & key : sqlTableConfig->mKeys)
        {
            const FieldDescriptor *fieldDesc = pDescriptor->FindFieldByName(key);
            mSqlCommandStream2 << key << "=";
            if (fieldDesc->type() == FieldDescriptor::TYPE_STRING)
            {
                std::string key = pReflection->GetString(messageData, fieldDesc);
                if (key == fieldDesc->default_value_string())
                {
                    return false;
                }
                mSqlCommandStream2 << "'" << key << "'";
            }
            else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
            {
                long long key = pReflection->GetInt64(messageData, fieldDesc);
                if (key == fieldDesc->default_value_int64())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            }
            else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT64)
            {
                unsigned long long key = pReflection->GetUInt64(messageData, fieldDesc);
                if (key == fieldDesc->default_value_uint64())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            }
            else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
            {
                int key = pReflection->GetInt32(messageData, fieldDesc);
                if (key == fieldDesc->default_value_int32())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            }
            else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
            {
                unsigned int key = pReflection->GetUInt32(messageData, fieldDesc);
                if (key == fieldDesc->default_value_uint32())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            }
            else
            {
                return false;
            }
            if (sqlTableConfig->mKeys.size() > 1 && index < sqlTableConfig->mKeys.size() - 1)
            {
                mSqlCommandStream2 << " and ";
            }
            index++;
        }

        for (auto fieldDesc : fieldList)
        {
            if (sqlTableConfig->HasKey(fieldDesc->name()))
            {
                continue;
            }
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

    bool MysqlComponent::GetQuerySqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand)
    {
        const std::string typeName = messageData.GetTypeName();
        const SqlTableConfig * sqlTableConfig = this->GetCondifByProto(typeName);
        if(sqlTableConfig == nullptr)
        {
            LOG_ERROR("get sql config by " << typeName << " failure");
            return false;
        }

        db = sqlTableConfig->mDb;
        mSqlCommandStream.str("");

        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();
        mSqlCommandStream << "select * from " << sqlTableConfig->mTableName << " where ";
        std::vector<const FieldDescriptor *> fieldList;
        pReflection->ListFields(messageData, &fieldList);
        if (fieldList.empty())
        {
            return false;
        }
        for (size_t index = 0; index < fieldList.size(); index++)
        {
            const FieldDescriptor *fieldDesc = fieldList[index];
            const std::string &key = fieldDesc->name();

            if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
            {
                const std::string val = pReflection->GetString(messageData, fieldDesc);
                if (val == fieldDesc->default_value_string())
                {
                    continue;
                }
                mSqlCommandStream << key << "='" << val << "'";
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
            {
                long long val = pReflection->GetInt64(messageData, fieldDesc);
                if (val == fieldDesc->default_value_int64())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            }
            else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
            {
                unsigned long long val = pReflection->GetUInt64(messageData, fieldDesc);
                if (val == fieldDesc->default_value_uint64())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            }
            else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
            {
                int val = pReflection->GetInt32(messageData, fieldDesc);
                if (val == fieldDesc->default_value_int32())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            }
            else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
            {
                unsigned int val = pReflection->GetUInt32(messageData, fieldDesc);
                if (val == fieldDesc->default_value_uint32())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            }
            if (sqlTableConfig->mKeys.size() > 1 && index < fieldList.size() - 1)
            {
                mSqlCommandStream << " and ";
            }
        }
        sqlCommand = mSqlCommandStream.str();
        return true;
    }

    bool MysqlComponent::GetDeleteSqlCommand(const Message &messageData,std::string & db, std::string &sqlCommand)
    {
        const std::string typeName = messageData.GetTypeName();
        const SqlTableConfig * sqlTableConfig = this->GetCondifByProto(typeName);
        if(sqlTableConfig == nullptr)
        {
            LOG_ERROR("get sql config by " << typeName << " failure");
            return false;
        }

        db = sqlTableConfig->mDb;
        this->mSqlCommandStream.str("");
        mSqlCommandStream << "delete from " << sqlTableConfig->mTableName << " where ";
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();

        int index = 0;
        for(const std::string & key : sqlTableConfig->mKeys)
        {
            const FieldDescriptor *fieldDesc = pDescriptor->FindFieldByName(key);
            mSqlCommandStream << key << "=";
            if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
            {
                const std::string key = pReflection->GetString(messageData, fieldDesc);
                if (key == fieldDesc->default_value_string())
                {
                    return false;
                }
                mSqlCommandStream << "'" << key << "'";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
            {
                int key = pReflection->GetInt32(messageData, fieldDesc);
                if (key == fieldDesc->default_value_int32())
                {
                    return false;
                }
                mSqlCommandStream << key;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
            {
                unsigned int key = pReflection->GetUInt32(messageData, fieldDesc);
                if (key == fieldDesc->default_value_uint32())
                {
                    return false;
                }
                mSqlCommandStream << key;
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
            {
                long long key = pReflection->GetInt64(messageData, fieldDesc);
                if (key == fieldDesc->default_value_int64())
                {
                    return false;
                }
                mSqlCommandStream << key;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT64)
            {
                unsigned long long key = pReflection->GetUInt64(messageData, fieldDesc);
                if (key == fieldDesc->default_value_uint64())
                {
                    return false;
                }
                mSqlCommandStream << key;
            } else
            {
                return false;
            }

            if (sqlTableConfig->mKeys.size() > 1 && index < sqlTableConfig->mKeys.size() - 1)
            {
                mSqlCommandStream << " and ";
            }
            index++;
        }
        sqlCommand = mSqlCommandStream.str();
        return true;
    }
}