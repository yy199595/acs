#include"MysqlTaskProxy.h"
#include<Core/App.h>
#include"Component/MysqlComponent.h"
#include<Coroutine/TaskComponent.h>
#include"Util/JsonHelper.h"

namespace GameKeeper
{
    MysqlTaskProxy::MysqlTaskProxy(std::string db, std::string sql)
        : mDataBaseName(std::move(db)), mSqlCommand(std::move(sql))
    {	
		TaskComponent * corComponent = App::Get().GetTaskComponent();
        this->mCoroutineId = corComponent->GetCurrentCorId();
        this->mMsqlComponent = App::Get().GetComponent<MysqlComponent>();

    }

    void MysqlTaskProxy::RunFinish()
    {
		TaskComponent * corComponent = App::Get().GetTaskComponent();
        if (this->mCoroutineId != 0)
        {
			corComponent->Resume(this->mCoroutineId);
        }
        if(this->mErrorCode != XCode::Successful)
        {
            LOG_ERROR(this->mSqlCommand);
            LOG_ERROR(this->mErrorString);
        }
    }

    bool MysqlTaskProxy::Run()
    {
        GKMysqlSocket *mysqlSocket = this->mMsqlComponent->GetMysqlSocket();
        if (mysqlSocket == nullptr)
        {
            this->mErrorCode = MysqlSocketIsNull;
            this->mErrorString = "mysql socket is null";
            return true;
        }

        if (mysql_select_db(mysqlSocket, this->mDataBaseName.c_str()) != 0)
        {
            this->mErrorCode = MysqlSelectDbFailure;
            this->mErrorString = "select " + this->mDataBaseName + " fail";
            return true;
        }

        if (mysql_real_query(mysqlSocket, mSqlCommand.c_str(), mSqlCommand.size()) != 0)
        {
            this->mErrorCode = MysqlInvokeFailure;
            this->mErrorString = mysql_error(mysqlSocket);
            return true;
        }
        this->mErrorCode = XCode::Successful;
        MysqlQueryResult *queryResult = mysql_store_result(mysqlSocket);
        if (queryResult != nullptr)
        {
            std::string json;
            std::vector<MYSQL_FIELD *> fieldNameVector;
            unsigned long rowCount = mysql_num_rows(queryResult);
            unsigned int fieldCount = mysql_field_count(mysqlSocket);
            for (unsigned int index = 0; index < fieldCount; index++)
            {
                MYSQL_FIELD *field = mysql_fetch_field(queryResult);
                fieldNameVector.push_back(field);
            }
            if (rowCount == 1)
            {
                RapidJsonWriter jsonWrite;
                MYSQL_ROW row = mysql_fetch_row(queryResult);
                unsigned long *lengths = mysql_fetch_lengths(queryResult);
                for (size_t index = 0; index < fieldNameVector.size(); index++)
                {
                    MYSQL_FIELD *field = fieldNameVector[index];
                    this->WriteValue(jsonWrite, field, row[index], (int) lengths[index]);
                }
                if (jsonWrite.WriterToStream(json))
                {
                    this->mQueryDatas.push(json);
                }
            }
            else
            {
                for (unsigned long count = 0; count < rowCount; count++)
                {
                    RapidJsonWriter jsonWrite;
                    MYSQL_ROW row = mysql_fetch_row(queryResult);
                    unsigned long *lengths = mysql_fetch_lengths(queryResult);
                    for (size_t index = 0; index < fieldNameVector.size(); index++)
                    {
                        MYSQL_FIELD *field = fieldNameVector[index];
                        this->WriteValue(jsonWrite, field, row[index], (int) lengths[index]);
                    }
                    if (jsonWrite.WriterToStream(json))
                    {
                        this->mQueryDatas.push(json);
                    }
                }
            }
            mysql_free_result(queryResult);
        }
        return true;
    }

    bool MysqlTaskProxy::GetQueryData(std::string &data)
    {
        if(this->mQueryDatas.empty())
        {
            return false;
        }
        data = this->mQueryDatas.front();
        this->mQueryDatas.pop();
        return true;
    }

    void MysqlTaskProxy::WriteValue(RapidJsonWriter &jsonWriter, MYSQL_FIELD *field, const char *data, long size)
    {
        switch (field->type)
        {
            case enum_field_types::MYSQL_TYPE_LONG:
            case enum_field_types::MYSQL_TYPE_LONGLONG:
                this->mValue2 = std::atoll(data);
                if (this->mValue2 != 0)
                {
                    jsonWriter.Add(field->name, this->mValue2);
                }
                break;
            case enum_field_types::MYSQL_TYPE_FLOAT:
            case enum_field_types::MYSQL_TYPE_DOUBLE:
                this->mValue1 = std::atof(data);
                if (this->mValue1 != 0)
                {
                    jsonWriter.Add(field->name, this->mValue1);
                }
            default:
                if (data != nullptr && size > 0)
                {
                    jsonWriter.Add(field->name, data, size);
                }
                break;
        }
    }
}// namespace GameKeeper