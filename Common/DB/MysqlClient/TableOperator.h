#pragma once

#include<string>
#include"MysqlComponent.h"
#include"rapidjson/document.h"
namespace Sentry
{

    /*
    根据protobuf文件创建sql表
    如果字段没有会更新字段
    */

    class TableOperator
    {
    public:
        TableOperator(MysqlClient *socket);

    public:
        bool InitDb(const std::string & name);
        bool InitTable(const Descriptor * descriptor);

    private:
        bool CreateMysqlTable(const Descriptor * descriptor);

        bool UpdateMysqlTable(const Descriptor * descriptor);

    private:
        bool AddNewField(const std::string& table, const FieldDescriptor *fieldDesc);

    private:
        MysqlClient *mMysqlSocket;
    };
}