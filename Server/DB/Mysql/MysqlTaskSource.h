#pragma once
#include<queue>
#include"MysqlDefine.h"
#include"Json/JsonWriter.h"
#include"Async/TaskSource.h"
namespace Sentry
{

    class TaskComponent;
	class MysqlTaskSource : public MysqlAsyncTask
    {
    public:
        explicit MysqlTaskSource(const std::string & sql, s2s::Mysql::Response & response);
    public:
		XCode Await() final;
		void Run(MysqlSocket * mysql) final;
    private:
        void WriteValue(Json::Writer &jsonWriter, MYSQL_FIELD *field, const char *data, long size);
    private:
        TaskSource<XCode> mTaskSource;
		const std::string mSqlCommand;
		s2s::Mysql::Response & mResponse;
    };
}