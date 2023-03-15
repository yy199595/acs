//
// Created by zmhy0073 on 2022/8/25.
//
#ifdef __ENABLE_MYSQL__
#ifndef APP_MYSQLMESSAGE_H
#define APP_MYSQLMESSAGE_H
#ifdef _WIN32
#include<WinSock2.h>
#include<windows.h>
#endif
#include"mysql.h"
#include<memory>
#include<string>
#include<sstream>
#include<vector>
#include"Json/JsonWriter.h"
#include"google/protobuf/message.h"
using namespace google::protobuf;
namespace Mysql
{
	class Response : public std::vector<Json::Writer *>
	{
    public:
        Response(int taskId)
            : mTaskId(taskId) ,mIsOk(true) { }
    public:
		void SetError(const char * str);
		bool IsOk() const { return this->mIsOk;}
		int TaskId() const { return this->mTaskId; }
        const std::string & GetError() const { return this->mError;}
    private:
        bool mIsOk;
		int mTaskId;
        std::string mError;
	};
    class ICommand
	{
	 public:
        ICommand() : mTaskId(0) { }
		virtual ~ICommand() { }
        virtual bool Invoke(MYSQL *, std::shared_ptr<Response> & response) = 0;
	public:
		int GetRpcId() const { return this->mTaskId; }
		void SetRpcId(int id) { this->mTaskId = id;}
	private:
		int mTaskId;
	};

    class PingCommand : public ICommand
    {
    public:
        using ICommand::ICommand;
        bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
    };

	class SqlCommand : public ICommand
	{
	 public:
        SqlCommand(const std::string & sql);
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        const std::string mSql;
	};

 	class QueryCommand : public ICommand
    {
    public:
        QueryCommand(const std::string & sql);
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        bool Write(Json::Writer & document, st_mysql_field * filed, const char * str, int len);
    private:
        const std::string mSql;
    };
}

namespace Mysql
{
    class CreateTabCommand : public ICommand
    {
    public:
        CreateTabCommand(std::shared_ptr<Message> message, std::vector<std::string> & keys);
    public:
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        void ClearBuffer();
        bool ForeachMessage(const FieldDescriptor * field);
        bool CreateTable(MYSQL * sock, const std::string & tab, std::string & eror);
        bool CheckTableField(MYSQL * sock, const std::string & tab, std::string & error);
        bool AddNewField(MYSQL * sock, const std::string & tab, const std::string & field, std::string & error);
    private:
        std::stringstream mBuffer;
        std::vector<std::string> mKeys;
        std::shared_ptr<Message> mMessage;
    };
}

namespace Mysql
{
    class SetMainKeyCommand : public ICommand
    {
    public:
        SetMainKeyCommand(const std::string & tab, std::vector<std::string> & keys);
    public:
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        std::string mTable;
        std::vector<std::string> mKeys;
    };
}


#endif //APP_MYSQLMESSAGE_H
#endif
