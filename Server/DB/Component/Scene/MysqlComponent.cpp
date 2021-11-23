#include "MysqlComponent.h"
#include <fstream>
#include <utility>
#include <Core/App.h>
#include <Scene/TaskPoolComponent.h>
#include <MysqlClient/TableOperator.h>
#include <Coroutine/CoroutineComponent.h>

namespace GameKeeper
{
    SqlTableConfig::SqlTableConfig(std::string  tab, const std::string pb)
            : mTableName(std::move(tab)), mProtobufName(pb)
    {
    }

    void SqlTableConfig::AddKey(const std::string key)
    {
        this->mKeys.push_back(key);
    }

    bool SqlTableConfig::HasKey(const std::string &key) const
    {
        for (const std::string &k : this->mKeys)
        {
            if (k == key)
            {
                return true;
            }
        }
        return false;
    }
}

namespace GameKeeper
{
    MysqlComponent::MysqlComponent()
    {
        this->mMysqlPort = 0;
    }

    bool MysqlComponent::Awake()
    {
		const ServerConfig & config = App::Get().GetConfig();
		this->mSqlPath = App::Get().GetConfigPath() + "sql.json";
        GKAssertRetFalse_F(this->mTaskManager = this->GetComponent<TaskPoolComponent>());
        GKAssertRetFalse_F(this->mCorComponent = this->GetComponent<CoroutineComponent>());

        GKAssertRetFalse_F(config.GetValue("Mysql", "ip", this->mMysqlIp));
        GKAssertRetFalse_F(config.GetValue("Mysql", "port",this->mMysqlPort));
		GKAssertRetFalse_F(config.GetValue("Mysql", "db", this->mDataBaseName));
        GKAssertRetFalse_F(config.GetValue("Mysql","user",this->mDataBaseUser));    
		GKAssertRetFalse_F(config.GetValue("Mysql", "passwd", this->mDataBasePasswd));

        GKAssertRetFalse_F(this->StartConnect());
        GKAssertRetFalse_F(this->InitMysqlTable());
        return true;
    }

    GKMysqlSocket *MysqlComponent::GetMysqlSocket()
    {
        if(App::Get().IsMainThread())
        {
            GKDebugError("try in main thread invoke sql");
            return nullptr;
        }
		auto id = std::this_thread::get_id();
        auto iter = this->mMysqlSocketMap.find(id);
        return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
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
			GKDebugError("connect mysql failure "
				<< ip << ":" << port << "  " << userName << " " << passWd);
			return nullptr;
		}
		GKDebugInfo("connect mysql successful [" << ip << ":" << port << "]");
		return this->mMysqlSockt;
	}

    bool MysqlComponent::GetTableName(const std::string &pb, std::string &table)
    {
        auto iter = this->mTablePbMap.find(pb);
        if (iter != this->mTablePbMap.end())
        {
            table = iter->second;
            return true;
        }
        return false;
    }

    const SqlTableConfig *MysqlComponent::GetTableConfig(const std::string &tab) const
    {
        auto iter = this->mSqlConfigMap.find(tab);
        return iter != this->mSqlConfigMap.end() ? iter->second : nullptr;
    }


    void MysqlComponent::Start()
    {

    }

    bool MysqlComponent::InitMysqlTable()
    {
        std::fstream fs(this->mSqlPath, std::ios::in);
        if (!fs.is_open())
        {
            GKDebugError("not find file " << this->mSqlPath);
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
            GKDebugError("parse " << mSqlPath << " json fail");
            return false;
        }
        for (auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
        {
            if (!iter->name.IsString() || !iter->value.IsObject())
            {
                GKDebugError(mSqlPath << " error");
                return false;
            }
            auto iter1 = iter->value.FindMember("keys");
            auto iter2 = iter->value.FindMember("protobuf");
            if (iter1 == iter->value.MemberEnd() || !iter1->value.IsArray())
            {
                GKDebugError(mSqlPath << " error");
				return false;
            }
            if (iter2 == iter->value.MemberEnd() || !iter2->value.IsString())
            {
                GKDebugError(mSqlPath << " error");
				return false;
            }
            const std::string tab = iter->name.GetString();
            const std::string pb = iter2->value.GetString();
            auto tabConfig = new SqlTableConfig(tab, pb);
            for (unsigned int index = 0; index < iter1->value.Size(); index++)
            {
                tabConfig->AddKey(iter1->value[index].GetString());
            }
            this->mTablePbMap.emplace(pb, tab);
            this->mSqlConfigMap.emplace(tab, tabConfig);
        }
        TableOperator tableOper(this->mMysqlSockt, this->mDataBaseName, document);
        return tableOper.InitMysqlTable();
    }

    bool MysqlComponent::StartConnect()
    {
		const std::vector<TaskThread *> & threadTasks = this->mTaskManager->GetThreads();
		for (TaskThread * taskThread : threadTasks)
		{
			auto mysqlScoket = this->ConnectMysql();
			if(mysqlScoket == nullptr)
            {
                return false;
            }
			this->mMysqlSocketMap.insert(std::make_pair(taskThread->GetThreadId(), mysqlScoket));
		}
        return true;
    }

    bool MysqlComponent::GetTableNameByProtocolName(const std::string &name, std::string &tableName)
    {
        auto iter = this->mTablePbMap.find(name);
        if (iter != this->mTablePbMap.end())
        {
            tableName = iter->second;
            return true;
        }
        return false;
    }

    bool MysqlComponent::GetAddSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        GKAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        this->mSqlCommandStream.str("");

        std::vector<const FieldDescriptor *> fieldList;
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();
        pReflection->ListFields(messageData, &fieldList);

        mSqlCommandStream << "insert into " << tableName << "(";
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

    bool MysqlComponent::GetSaveSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        GKAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        const SqlTableConfig *tableConfig = this->GetTableConfig(tableName);
        GKAssertRetFalse_F(tableConfig);


        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();


        std::vector<const FieldDescriptor *> fieldList;
        pReflection->ListFields(messageData, &fieldList);

        mSqlCommandStream2 << " where ";
        mSqlCommandStream << "update " << tableName << " set ";

        for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
        {
            const std::string &key = tableConfig->mKeys[index];
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
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
            {
                long long key = pReflection->GetInt64(messageData, fieldDesc);
                if (key == fieldDesc->default_value_int64())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT64)
            {
                unsigned long long key = pReflection->GetUInt64(messageData, fieldDesc);
                if (key == fieldDesc->default_value_uint64())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
            {
                int key = pReflection->GetInt32(messageData, fieldDesc);
                if (key == fieldDesc->default_value_int32())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
            {
                unsigned int key = pReflection->GetUInt32(messageData, fieldDesc);
                if (key == fieldDesc->default_value_uint32())
                {
                    return false;
                }
                mSqlCommandStream2 << key;
            } else
            {
                return false;
            }
            if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
            {
                mSqlCommandStream2 << " and ";
            }
        }

        for (auto fieldDesc : fieldList)
        {
            if (tableConfig->HasKey(fieldDesc->name()))
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

    bool MysqlComponent::GetQuerySqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        GKAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        const SqlTableConfig *tableConfig = this->GetTableConfig(tableName);
        GKAssertRetFalse_F(tableConfig);

        mSqlCommandStream.str("");

        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();
        mSqlCommandStream << "select * from " << tableName << " where ";
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
            if (tableConfig->mKeys.size() > 1 && index < fieldList.size() - 1)
            {
                mSqlCommandStream << " and ";
            }
        }
        sqlCommand = mSqlCommandStream.str();
        return true;
    }

    bool MysqlComponent::GetDeleteSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        GKAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        const SqlTableConfig *tableConfig = this->GetTableConfig(tableName);
        GKAssertRetFalse_F(tableConfig);

        this->mSqlCommandStream.str("");


        mSqlCommandStream << "delete from " << tableName << " where ";
        const Descriptor *pDescriptor = messageData.GetDescriptor();
        const Reflection *pReflection = messageData.GetReflection();

        for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
        {
            const std::string &key = tableConfig->mKeys[index];
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

            if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
            {
                mSqlCommandStream << " and ";
            }
        }
        sqlCommand = mSqlCommandStream.str();
        return true;
    }
}