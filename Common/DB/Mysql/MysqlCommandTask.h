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
			XCode Await() final;
        protected:
			virtual XCode OnInvoke() = 0;
			void Run(MysqlSocket* mysql) final;
			bool Invoke(const std::string & sql, std::string & error);
			inline MysqlSocket * GetMysqlSocket() { return this->mMysqlSocket; }
			MysqlQueryResult * InvokeQuery(const std::string & sql, std::string & eror);
		 protected:
			std::string mError;
			std::string mCommand;
		 private:
			MysqlSocket * mMysqlSocket;
			TaskSource<XCode> mTaskSource;
			std::queue<MysqlQueryResult *> mResults;
		};
	}

	namespace Mysql
	{
		class MysqlAddCommandTask : public MysqlCommandTask
		{
		public:
			MysqlAddCommandTask(const s2s::Mysql::Add & request)
				: mRequest(request) { }
		 private:
			bool Init();
			XCode OnInvoke() final;
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
		 private:
			bool Init();
			XCode OnInvoke() final;
		private:
			const s2s::Mysql::Save& mRequest;
		};
	}

	namespace Mysql
	{
		class MysqlQueryCommandTask : public MysqlCommandTask
		{
		public:
			MysqlQueryCommandTask(const s2s::Mysql::Query& request, s2s::Mysql::Response & response, Message & message)
				: mRequest(request), mResponse(response), mTempMessage(message) { }
		private:
			bool Init() final;
			XCode OnInvoke() final;
			bool WriteValue(MYSQL_FIELD *field, const char *data, long size);
		private:
			Message & mTempMessage;
			s2s::Mysql::Response & mResponse;
			const s2s::Mysql::Query & mRequest;
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
			bool Init() final;
			XCode OnInvoke() final;
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
			bool Init() final;
			XCode OnInvoke() final;
		private:
			const s2s::Mysql::Delete & mRequest;
		};
	}
}


#endif //SERVER_MYSQLCOMMANDTASK_H
