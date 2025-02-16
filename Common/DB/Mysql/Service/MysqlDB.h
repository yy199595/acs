#pragma once

#include"Util/Sql/SqlHelper.h"
#include"Rpc/Service/RpcService.h"

namespace acs
{
	class MysqlDB : public RpcService, public IDestroy
	{
	 public:
		MysqlDB();
	private:
		int Index(const db::mysql::index & request);
		int Create(const db::mysql::create& request);
		int Save(const db::mysql::save& request, db::mysql::response &response);
		int Delete(const db::mysql::del& request, db::mysql::response &response);
		int Insert(const db::mysql::insert& request, db::mysql::response &response);
		int Update(const db::mysql::update& request, db::mysql::response & response);
		int Exec(const db::mysql::exec& request, db::mysql::query::response & response);
        int Query(const db::mysql::query::request & request, db::mysql::query::response & response);
		int FindPage(const db::mysql::query::page & request, db::mysql::query::response & response);
	 private:
		bool Awake() final;
		bool OnInit() final;
		void OnDestroy() final;
	private:
		SqlHelper mSqlHelper;
		class ProtoComponent * mProto;
		class MysqlDBComponent * mMysql;
        std::unordered_map<std::string, std::string> mMainKeys;
    };
}// namespace Sentry
