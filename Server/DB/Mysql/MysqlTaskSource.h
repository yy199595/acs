#pragma once
#include<queue>
#include"MysqlDefine.h"
#include"Json/JsonWriter.h"
#include"Async/TaskSource.h"
namespace Sentry
{
    class TaskComponent;
	class MysqlTaskSource
    {
    public:
        explicit MysqlTaskSource(const std::string & sql);
    public:
		void Run(MysqlSocket * mysql);
        XCode Await(s2s::Mysql::Response & response);
    private:
        void WriteValue(Json::Writer &jsonWriter, MYSQL_FIELD *field, const char *data, long size);
    private:
        TaskSource<XCode> mTaskSource;
		const std::string mSqlCommand;
		s2s::Mysql::Response mResponse;
    };
}