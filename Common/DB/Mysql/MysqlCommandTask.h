//
// Created by mac on 2022/5/20.
//

#ifndef SERVER_MYSQLCOMMANDTASK_H
#define SERVER_MYSQLCOMMANDTASK_H
#include"MysqlDefine.h"
#include"Async/TaskSource.h"
namespace Sentry
{
	namespace Mysql
	{
		class MysqlCommandTask : public MysqlAsyncTask
		{
		public:
			MysqlCommandTask() = default;
			virtual ~MysqlCommandTask() = default;
		public:
			XCode Init() final;
			XCode Await() final;
			const std::string & GetError() { return this->mError;}
			const std::string & GetSql() { return this->mCommand;}
		protected:
			void Run(MysqlSocket* mysql) final;
			virtual bool GetCommand(MysqlHelper & helper, std::string & sql) = 0;
			virtual void OnComplete(MysqlSocket* mysql, MysqlQueryResult * result) {};
		private:
			std::string mError;
			std::string mCommand;
			TaskSource<XCode> mTaskSource;
		};
	}

	namespace Mysql
	{
		class MysqlAddCommandTask : public MysqlCommandTask
		{
		public:
			MysqlAddCommandTask(const s2s::Mysql::Add & request)
				: mRequest(request) { }
		protected:
			bool GetCommand(MysqlHelper &helper, std::string &sql) final;
		private:
			const s2s::Mysql::Add & mRequest;
		};
	}

	namespace Mysql
	{
		class MysqlSaveCommandTask : public MysqlCommandTask
		{
		public:
			MysqlSaveCommandTask(const s2s::Mysql::Save& request)
				: mRequest(request) { }
		protected:
			bool GetCommand(MysqlHelper &helper, std::string &sql) final;
		private:
			const s2s::Mysql::Save& mRequest;
		};
	}

	namespace Mysql
	{
		class MysqlQueryCommandTask : public MysqlCommandTask
		{
		public:
			MysqlQueryCommandTask(const s2s::Mysql::Query& request, s2s::Mysql::Response & response)
				: mRequest(request), mResponse(response) { }
		private:
			bool GetCommand(MysqlHelper &helper, std::string &sql) final;
			void OnComplete(MysqlSocket* mysql, MysqlQueryResult *result) final;
			void WriteValue(Json::Writer &jsonWriter, MYSQL_FIELD *field, const char *data, long size);
		private:
			const s2s::Mysql::Query & mRequest;
			s2s::Mysql::Response & mResponse;
		};
	}

	namespace Mysql
	{
		class MysqlUpdateCommandTask : public MysqlCommandTask
		{
		public:
			MysqlUpdateCommandTask(const s2s::Mysql::Update & request)
				: mRequest(request) {}

		private:
			bool GetCommand(MysqlHelper &helper, std::string &sql) final;
		private:
			const s2s::Mysql::Update & mRequest;
		};
	}

	namespace Mysql
	{
		class MysqlDeleteCommandTask : public MysqlCommandTask
		{
		public:
			MysqlDeleteCommandTask(const s2s::Mysql::Delete & request)
				: mRequest(request) { }
		private:
			bool GetCommand(MysqlHelper &helper, std::string &sql) final;
		private:
			const s2s::Mysql::Delete & mRequest;
		};
	}

	namespace Mysql
	{
		class MysqlIndexCommandTask : public MysqlCommandTask
		{
		 public:
			MysqlIndexCommandTask(const s2s::Mysql::Index & request)
				: mRequest(request) {}
		 public:
			bool GetCommand(MysqlHelper & helper, std::string & sql) final;
		 private:
			const s2s::Mysql::Index & mRequest;
		};
	}
}


#endif //SERVER_MYSQLCOMMANDTASK_H
