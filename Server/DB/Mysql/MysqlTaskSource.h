#pragma once
#include<queue>
#include"MysqlDefine.h"
#include"Json/JsonWriter.h"
#include"Async/TaskSource.h"
#include"Thread/TaskProxy.h"
namespace Sentry
{
    class SqlTableConfig;
    class TaskComponent;

	class MysqlTaskSource : public TaskProxy
    {
    public:
        MysqlTaskSource(MysqlHelper & component);

        ~MysqlTaskSource() final = default;

    protected:
        bool Run() final; //在其他线程查询
    public:
        XCode Await(const std::string & sql);

        const std::string &GetErrorStr() { return this->mErrorString; }

        bool GetQueryData(std::string & data);

    private:
        void WriteValue(Json::Writer &jsonWriter, MYSQL_FIELD *field, const char *data, long size);
    private:
		MysqlHelper & mHelper;
        std::string mSqlCommand;
        std::string mErrorString;
        TaskSource<XCode> mTaskSource;
        std::queue<std::string> mQueryDatas;
    private:
        double mValue1;
        long long mValue2;
    };
}