#pragma once

#include"MysqlDefine.h"
#include<queue>
#include <Thread/TaskProxy.h>
namespace GameKeeper
{
    class SqlTableConfig;

    class RapidJsonWriter;

    class CoroutineComponent;

    class MysqlTaskProxy : public TaskProxy
    {
    public:
        MysqlTaskProxy(std::string db, std::string sql);

        ~MysqlTaskProxy() final = default;

    protected:
        void RunFinish() final;

        bool Run() final; //在其他线程查询
    public:
        XCode GetErrorCode() { return this->mErrorCode; }

        const std::string &GetErrorStr() { return this->mErrorString; }

        bool GetQueryData(std::string & data);

    private:
        void WriteValue(RapidJsonWriter &jsonWriter, MYSQL_FIELD *field, const char *data, long size);

    private:
        const std::string mSqlCommand;
        const std::string mDataBaseName;
    private:
        XCode mErrorCode;
        long long mCoroutineId;
        std::string mErrorString;
        MysqlComponent * mMsqlComponent;
        std::queue<std::string> mQueryDatas;
    private:
        double mValue1;
        long long mValue2;
    };
}