#pragma once

#include "MysqlDefine.h"
#include <Thread/TaskProxy.h>

namespace Sentry
{
    class SqlTableConfig;

    class QuertJsonWritre;

    class CoroutineComponent;

    class MyqslTask : public TaskProxy
    {
    public:
        MyqslTask(const std::string &db, const std::string &sql);

        ~MyqslTask() {}

    protected:
        void RunFinish() final;

        void Run() final; //在其他线程查询
    public:
        XCode GetErrorCode() { return this->mErrorCode; }

        const std::string &GetErrorStr() { return this->mErrorString; }

        const std::vector<std::string> &GetQueryDatas() { return this->mQueryDatas; }

    private:
        void WriteValue(QuertJsonWritre &jsonWriter, MYSQL_FIELD *field, const char *data, long size);

    private:
        const std::string mSqlCommand;
        const std::string mDataBaseName;
    private:
        XCode mErrorCode;
        long long mCoroutineId;
        std::string mErrorString;
        std::vector<std::string> mQueryDatas;
    private:
        double mValue1;
        long long mValue2;
    };
}