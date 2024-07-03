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
#include"Lua/Engine/Define.h"
#include"Proto/Include/Message.h"
#include"Yyjson/Document/Document.h"

namespace Mysql
{
	class Response : public ILuaWrite
	{
    public:
		Response() = default;
    public:
		void SetError(const char * str);
        int WriteToLua(lua_State* lua) const final;
		bool IsOk() const { return this->mError.empty();}
        const std::string & GetError() const { return this->mError;}
	public:
		std::string ToString();
		void Add(const std::string & json);
		void Add(const char * str, size_t len);
		size_t ArraySize() const { return this->mResults.size();}
		void Clear() { this->mError.clear(); this->mResults.clear(); }
		const std::string & Get(size_t index) const { return this->mResults[index]; }
		const std::vector<std::string> & Array() const { return this->mResults; }
    private:
        std::string mError;
		std::vector<std::string> mResults;
	};
    class IRequest
	{
	 public:
        IRequest();
		virtual ~IRequest() = default;
		virtual void GetSql(std::string & sql) { }
        virtual bool Invoke(MYSQL *,  Response * response) = 0;
	public:
		long long GetMs();
		int GetRpcId() const { return this->mTaskId; }
		void SetRpcId(int id) { this->mTaskId = id;}
	private:
		int mTaskId;
		long long mTime;
	};

	class StopRequest : public IRequest
	{
	public:
		using IRequest::IRequest;
		bool Invoke(MYSQL *, Response * response) final { return false; }
	};

    class PingRequest : public IRequest
    {
    public:
        using IRequest::IRequest;
        bool Invoke(MYSQL *, Response * response) final;
    };

	class SqlRequest : public IRequest
	{
	 public:
        explicit SqlRequest(const std::string & sql);
		bool Invoke(MYSQL *, Response * response) final;
		void GetSql(std::string &sql) final { sql = this->mSql; }
	private:
        const std::string mSql;
	};

 	class FindRequest : public IRequest
    {
    public:
        explicit FindRequest(const std::string & sql);
		void GetSql(std::string &sql) final { sql = this->mSql; }
	private:
		bool Invoke(MYSQL *, Response * response) final;
		static bool Write(json::w::Value & document, st_mysql_field * filed, const char * str, int len);
    private:
        const std::string mSql;
    };
}

namespace Mysql
{
    class MakeTabRequest : public IRequest
    {
    public:
        MakeTabRequest(std::string  table,
				std::unique_ptr<pb::Message> message, std::vector<std::string> & keys);
    public:
		bool Invoke(MYSQL *, Response * response) final;
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
        std::unique_ptr<pb::Message> mMessage;
    };
}

namespace Mysql
{
    class SetMainKeyRequest : public IRequest
    {
    public:
        SetMainKeyRequest(std::string  tab, std::vector<std::string> & keys);
    public:
		bool Invoke(MYSQL *, Response * response) final;
	private:
        std::string mTable;
        std::vector<std::string> mKeys;
    };
}


#endif //APP_MYSQLMESSAGE_H
#endif
