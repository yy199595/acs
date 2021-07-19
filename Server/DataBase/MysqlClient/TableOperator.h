#pragma once
#include<string>
#include<Manager/MysqlManager.h>
namespace Sentry
{

	/*
	根据protobuf文件创建sql表
	如果字段没有会更新字段
	*/

	class TableOperator
	{
	public:
		TableOperator(SayNoMysqlSocket * socket, const std::string db, rapidjson::Document & doc);
	public:
		bool InitMysqlTable();
	private:
		bool CreateMysqlTable(const std::string table, const std::string name, const std::vector<std::string> & keys);
		bool UpdateMysqlTable(const std::string table, const std::string name, const std::vector<std::string> & keys);
	private:
		bool AddNewField(const std::string table, const FieldDescriptor * fieldDesc);
	private:
		std::string mDataBase;
		rapidjson::Document & mDocument;
		SayNoMysqlSocket * mMysqlSocket;
		
	};
}