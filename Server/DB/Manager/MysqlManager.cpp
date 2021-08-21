#include "MysqlManager.h"
#include <fstream>
#include <Coroutine/CoroutineManager.h>
#include <Util/StringHelper.h>
#include <Script/ClassProxyHelper.h>

#include <MysqlClient/TableOperator.h>
#include<Manager/ThreadTaskManager.h>

namespace Sentry
{
    SqlTableConfig::SqlTableConfig(const std::string tab, const std::string pb)
            : mTableName(tab), mProtobufName(pb)
    {
    }

    void SqlTableConfig::AddKey(const std::string key)
    {
        this->mKeys.push_back(key);
    }

    bool SqlTableConfig::HasKey(const std::string &key)
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

namespace Sentry
{
    MysqlManager::MysqlManager()
    {
        this->mMysqlPort = 0;
    }

    bool MysqlManager::OnInit()
    {
        SayNoAssertRetFalse_F(this->mTaskManager = this->GetManager<ThreadTaskManager>());
        SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Mysql", "ip", this->mMysqlIp));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Mysql", "port",this->mMysqlPort));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Mysql","user",this->mDataBaseUser));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Mysql","passwd",this->mDataBasePasswd));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Mysql","db",this->mDataBaseName));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Mysql", "sql", this->mSqlTablePath));

        SayNoAssertRetFalse_F(this->StartConnectMysql());
        SayNoAssertRetFalse_F(this->InitMysqlTable());
        return true;
    }

    SayNoMysqlSocket *MysqlManager::GetMysqlSocket(long long threadId)
    {
        auto iter = this->mMysqlSocketMap.find(threadId);
        return iter != this->mMysqlSocketMap.end() ? iter->second : nullptr;
    }

    bool MysqlManager::GetTableName(const std::string &pb, std::string &table)
    {
        auto iter = this->mTablePbMap.find(pb);
        if (iter != this->mTablePbMap.end())
        {
            table = iter->second;
            return true;
        }
        return false;
    }

    SqlTableConfig *MysqlManager::GetTableConfig(const std::string &tab)
    {
        auto iter = this->mSqlConfigMap.find(tab);
        return iter != this->mSqlConfigMap.end() ? iter->second : nullptr;
    }


    void MysqlManager::OnInitComplete()
    {
        /*this->mCoroutineManager->Start("MysqlProxy", [this]()
            {
                ServiceManager * serviceManager = this->GetManager<ServiceManager>();

                LocalService * mysqlService = serviceManager->GetLocalService("MysqlProxy");

                MysqlProxy * mysqlProxy = (MysqlProxy *)mysqlService;

                s2s::MysqlOper_Request requestData;
                s2s::MysqlOper_Response responseData;

                db::UserAccountData userData;
                userData.set_userid(13716061995);
                for (int index = 0; index < 10; index++)
                {
                    userData.set_phonenum(15510561831 + index);
                    userData.set_passwd("0x0x0x0x0x0x0x0x0x0x0x");
                    userData.set_token(TimeHelper::GetDateString());
                    userData.set_registertime(TimeHelper::GetSecTimeStamp());

                    userData.set_account(std::to_string((646585122 + index)) + "@qq.com");

                    requestData.set_protocolname(userData.GetTypeName());
                    requestData.set_protocolmessage(userData.SerializeAsString());

                    XCode code = mysqlProxy->Add(0, requestData, responseData);
                    SayNoDebugError("code = " << code);
                }

                userData.Clear();
                userData.set_userid(13716061995);

                s2s::MysqlQuery_Request queryRequest;
                s2s::MysqlQuery_Response queryResponse;
                queryRequest.set_protocolname(userData.GetTypeName());
                queryRequest.set_protocolmessage(userData.SerializeAsString());

                mysqlProxy->QueryData(0, queryRequest, queryResponse);
            });*/



    }

    bool MysqlManager::InitMysqlTable()
    {
        std::fstream fs(this->mSqlTablePath, std::ios::in);
        if (!fs.is_open())
        {
            SayNoDebugError("not find file " << this->mSqlTablePath);
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
            SayNoDebugError("parse " << mSqlTablePath << " json fail");
            return false;
        }
        for (auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
        {
            if (!iter->name.IsString() || !iter->value.IsObject())
            {
                SayNoDebugError(mSqlTablePath << " error");
                return false;
            }
            auto iter1 = iter->value.FindMember("keys");
            auto iter2 = iter->value.FindMember("protobuf");
            if (iter1 == iter->value.MemberEnd() || !iter1->value.IsArray())
            {
                SayNoDebugError(mSqlTablePath << " error");
            }
            if (iter2 == iter->value.MemberEnd() || !iter2->value.IsString())
            {
                SayNoDebugError(mSqlTablePath << " error");
            }
            const std::string tab = iter->name.GetString();
            const std::string pb = iter2->value.GetString();
            SqlTableConfig *tabConfig = new SqlTableConfig(tab, pb);
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

    bool MysqlManager::StartConnectMysql()
    {
        const char *ip = this->mMysqlIp.c_str();
        const unsigned short port = this->mMysqlPort;
        const char *passWd = this->mDataBasePasswd.c_str();
        const char *userName = this->mDataBaseUser.c_str();

        int count = this->mTaskManager->GetThreadCount();
        if (count == 0)
        {
            return false;
        }
        for (int index = 0; index < count; index++)
        {
            SayNoMysqlSocket *mysqlSocket1 = mysql_init((MYSQL *) 0);
            this->mMysqlSockt = mysql_real_connect(mysqlSocket1, ip, userName, passWd, NULL, port, NULL,
                                                   CLIENT_MULTI_STATEMENTS);
            if (this->mMysqlSockt == nullptr)
            {
                SayNoDebugError("connect mysql failure " << ip << ":" << port << "  " << userName << " " << passWd);
                return false;
            }
            this->mMysqlSocketMap.insert(std::make_pair(index, this->mMysqlSockt));
			SayNoDebugInfo("connect mysql successful [" << ip << ":" << port << "] [index = " << index << "]");
        }
       
        return true;
    }

    bool MysqlManager::GetTableNameByProtocolName(const std::string &name, std::string &tableName)
    {
        auto iter = this->mTablePbMap.find(name);
        if (iter != this->mTablePbMap.end())
        {
            tableName = iter->second;
            return true;
        }
        return false;
    }

    bool MysqlManager::GetAddSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        SayNoAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

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

        for (size_t index = 0; index < fieldList.size(); index++)
        {
            const FieldDescriptor *fieldDesc = fieldList[index];
            if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
            {
                mSqlCommandStream << "'" << pReflection->GetString(messageData, fieldDesc) << "',";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_BYTES)
            {
                mSqlCommandStream << "'" << pReflection->GetString(messageData, fieldDesc) << "',";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
            {
                mSqlCommandStream << pReflection->GetInt64(messageData, fieldDesc) << ",";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
            {
                mSqlCommandStream << pReflection->GetUInt64(messageData, fieldDesc) << ",";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
            {
                mSqlCommandStream << pReflection->GetInt32(messageData, fieldDesc) << ",";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT32)
            {
                mSqlCommandStream << pReflection->GetUInt32(messageData, fieldDesc) << ",";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_FLOAT)
            {
                mSqlCommandStream << pReflection->GetFloat(messageData, fieldDesc) << ",";
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_DOUBLE)
            {
                mSqlCommandStream << pReflection->GetDouble(messageData, fieldDesc) << ",";
            } else
            {
                return false;
            }
        }
        sqlCommand = mSqlCommandStream.str();
        sqlCommand[sqlCommand.size() - 1] = ')';
        return true;

    }

    bool MysqlManager::GetSaveSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        SayNoAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        SqlTableConfig *tableConfig = this->GetTableConfig(tableName);
        SayNoAssertRetFalse_F(tableConfig);


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

        for (size_t index = 0; index < fieldList.size(); index++)
        {
            const FieldDescriptor *fieldDesc = fieldList[index];
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

    bool MysqlManager::GetQuerySqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        SayNoAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        SqlTableConfig *tableConfig = this->GetTableConfig(tableName);
        SayNoAssertRetFalse_F(tableConfig);

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
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
            {
                long long val = pReflection->GetInt64(messageData, fieldDesc);
                if (val == fieldDesc->default_value_int64())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            } else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
            {
                unsigned long long val = pReflection->GetUInt64(messageData, fieldDesc);
                if (val == fieldDesc->default_value_uint64())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
            {
                int val = pReflection->GetInt32(messageData, fieldDesc);
                if (val == fieldDesc->default_value_int32())
                {
                    continue;
                }
                mSqlCommandStream << key << "=" << val;
            } else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
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

    bool MysqlManager::GetDeleleSqlCommand(const Message &messageData, std::string &sqlCommand)
    {
        std::string tableName;
        const std::string typeName = messageData.GetTypeName();
        SayNoAssertRetFalse_F(this->GetTableNameByProtocolName(typeName, tableName));

        SqlTableConfig *tableConfig = this->GetTableConfig(tableName);
        SayNoAssertRetFalse_F(tableConfig);

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