#pragma once
#ifdef __ENABLE_MYSQL__
#include"Util/Sql/SqlHelper.h"
#include"Mysql/Client/MysqlClient.h"
#include"Rpc/Service/RpcService.h"

namespace acs
{
	class MysqlDB : public RpcService
	{
	 public:
		MysqlDB();
	private:

        int Add(const db::mysql::add& request);
		int Save(const db::mysql::save& request);
		int Update(const db::mysql::update& request);
		int Delete(const db::mysql::remove& request);
        int Create(const db::mysql::create& request);
		int Exec(const db::mysql::exec& request, db::mysql::response& response);
        int Query(const db::mysql::query& request, db::mysql::response& response);

	 private:
		bool Awake() final;
		bool OnInit() final;
        void OnStop() final;
	private:
		SqlHelper mSqlHelper;
		class ProtoComponent * mProtoComponent;
        class MysqlDBComponent * mMysqlComponent;
        std::unordered_map<std::string, std::string> mMainKeys;
    };
}// namespace Sentry

#endif