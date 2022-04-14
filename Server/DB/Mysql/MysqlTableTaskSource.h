#pragma once

#include<string>
#include"MysqlDefine.h"
#include"Async/TaskSource.h"

namespace Sentry
{
    /*
    根据protobuf文件创建sql表
    如果字段没有会更新字段
    */
	class MysqlTableTaskSource : public MysqlAsyncTask
    {
	 public:
		void Run(MysqlSocket *mysql) final;
	 public:
		XCode InitMysqlTable(const std::string & name);
    private:
		bool InitDb(MysqlSocket * mysql, const std::string & name);
		bool InitTable(MysqlSocket * mysql, const Descriptor * descriptor);
        bool CreateMysqlTable(MysqlSocket * mysql, const Descriptor * descriptor);
        bool UpdateMysqlTable(MysqlSocket * mysql, const Descriptor * descriptor);
		bool AddNewField(MysqlSocket * mysql, const std::string& table, const FieldDescriptor *fieldDesc);
	 private:
		std::string mName;
		TaskSource<XCode> mTaskSource;
    };
}