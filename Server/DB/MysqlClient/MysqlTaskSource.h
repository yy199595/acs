#pragma once
#include<queue>
#include"MysqlDefine.h"
#include"Async/TaskSource.h"
#include<Thread/TaskProxy.h>
namespace GameKeeper
{
    class SqlTableConfig;

    class RapidJsonWriter;

    class TaskComponent;

    class MysqlTaskSource : public TaskProxy
    {
    public:
        MysqlTaskSource(MysqlComponent * component);

        ~MysqlTaskSource() final = default;

    protected:
        bool Run() final; //在其他线程查询
    public:
        XCode Await(const std::string & db, const std::string & sql);

        const std::string &GetErrorStr() { return this->mErrorString; }

        bool GetQueryData(std::string & data);

    private:
        void WriteValue(RapidJsonWriter &jsonWriter, MYSQL_FIELD *field, const char *data, long size);

    private:
         std::string mSqlCommand;
         std::string mDataBaseName;
    private:
        std::string mErrorString;
        TaskSource<XCode> mTaskSource;
        MysqlComponent * mMsqlComponent;
        std::queue<std::string> mQueryDatas;
    private:
        double mValue1;
        long long mValue2;
    };
}