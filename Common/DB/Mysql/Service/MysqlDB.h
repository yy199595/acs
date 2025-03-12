#pragma once

#include"Util/Sql/SqlHelper.h"
#include"Rpc/Service/RpcService.h"

namespace acs
{
	class MysqlDB : public RpcService
	{
	 public:
		MysqlDB();
	private:
		int Index(const db::sql::index & request);
		int Create(const db::sql::create& request, db::sql::response &response);
		int Save(const db::sql::save& request, db::sql::response &response);
		int Delete(const db::sql::del& request, db::sql::response &response);
		int Insert(const db::sql::insert& request, db::sql::response &response);
		int Update(const db::sql::update& request, db::sql::response & response);
		int Exec(const db::sql::exec& request, db::sql::query::response & response);
        int Query(const db::sql::query::request & request, db::sql::query::response & response);
		int FindPage(const db::sql::query::page & request, db::sql::query::response & response);
	 private:
		bool Awake() final;
		bool OnInit() final;
	private:
		SqlHelper mSqlHelper;
		class ProtoComponent * mProto;
		class MysqlDBComponent * mMysql;
    };
}// namespace Sentry
