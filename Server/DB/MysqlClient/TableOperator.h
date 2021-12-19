#pragma once

#include<string>
#include"Component/MysqlComponent.h"
#include<rapidjson/document.h>
namespace GameKeeper
{

    /*
    根据protobuf文件创建sql表
    如果字段没有会更新字段
    */

    class TableOperator
    {
    public:
        TableOperator(GKMysqlSocket *socket, std::string db, rapidjson::Document &doc);

    public:
        bool InitMysqlTable();

    private:
        bool CreateMysqlTable(const std::string& table, std::string name, const std::vector<std::string> &keys);

        bool UpdateMysqlTable(const std::string& table, std::string name, const std::vector<std::string> &keys);

    private:
        bool AddNewField(const std::string& table, const FieldDescriptor *fieldDesc);

    private:
        std::string mDataBase;
        rapidjson::Document &mDocument;
        GKMysqlSocket *mMysqlSocket;

    };
}