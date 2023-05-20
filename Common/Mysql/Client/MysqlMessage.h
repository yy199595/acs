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
#include"Util/Json/JsonWriter.h"
#include"Lua/Engine/Define.h"
#include"Proto/Include/Message.h"
namespace Mysql
{
	class Response : public ILuaWrite
	{
    public:
        explicit Response(int taskId)
            :mTaskId(taskId) { }
    public:
		void SetError(const char * str);
		int TaskId() const { return this->mTaskId; }
        int WriteToLua(lua_State* lua) const final;
		bool IsOk() const { return this->mError.empty();}
        const std::string & GetError() const { return this->mError;}
	public:
		void Add(const std::string & json);
		void Add(const char * str, size_t len);
		size_t ArraySize() const { return this->mResults.size();}
		const std::string & Get(size_t index) const { return this->mResults[index]; }
		const std::vector<std::string> & Array() const { return this->mResults; }
    private:
		int mTaskId;
        std::string mError;
		std::vector<std::string> mResults;
	};
    class ICommand
	{
	 public:
        ICommand() : mTaskId(0) { }
		virtual ~ICommand() = default;
		virtual void GetSql(std::string & sql) { }
        virtual bool Invoke(MYSQL *, std::shared_ptr<Response> & response) = 0;
	public:
		int GetRpcId() const { return this->mTaskId; }
		void SetRpcId(int id) { this->mTaskId = id;}
	private:
		int mTaskId;
	};

	class StopCommand : public ICommand
	{
	public:
		using ICommand::ICommand;
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final { return false; }
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
        explicit SqlCommand(const std::string & sql);
		void GetSql(std::string &sql) final { sql = this->mSql; }
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        const std::string mSql;
	};

 	class QueryCommand : public ICommand
    {
    public:
        explicit QueryCommand(const std::string & sql);
		void GetSql(std::string &sql) final { sql = this->mSql; }
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        static bool Write(Json::Writer & document, st_mysql_field * filed, const char * str, int len);
    private:
        const std::string mSql;
    };
}

namespace Mysql
{
    class CreateTabCommand : public ICommand
    {
    public:
        CreateTabCommand(std::string  table,
				std::shared_ptr<pb::Message> message, std::vector<std::string> & keys);
    public:
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        void ClearBuffer();
        bool ForeachMessage(const pb::FieldDescriptor * field);
        bool CreateTable(MYSQL * sock, const std::string & tab, std::string & eror);
        bool CheckTableField(MYSQL * sock, const std::string & tab, std::string & error);
        bool AddNewField(MYSQL * sock, const std::string & tab, const std::string & field, std::string & error);
    private:
		std::string mTable;
        std::stringstream mBuffer;
        std::vector<std::string> mKeys;
        std::shared_ptr<pb::Message> mMessage;
    };
}

namespace Mysql
{
    class SetMainKeyCommand : public ICommand
    {
    public:
        SetMainKeyCommand(std::string  tab, std::vector<std::string> & keys);
    public:
		bool Invoke(MYSQL *, std::shared_ptr<Response> & response) final;
	private:
        std::string mTable;
        std::vector<std::string> mKeys;
    };
}


#endif //APP_MYSQLMESSAGE_H
#endif
