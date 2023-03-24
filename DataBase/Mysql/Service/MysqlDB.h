#pragma once

#include"Helper/SqlHelper.h"
#include"Client/MysqlClient.h"
#include"Service/PhysicalRpcService.h"

namespace Sentry
{
	class MysqlDB : public PhysicalRpcService
	{
	 public:
		MysqlDB();
	private:

        int Add(const db::mysql::add& request);

		int Save(const db::mysql::save& request);

		int Update(const db::mysql::update& request);

		int Delete(const db::mysql::remove& request);

        int Create(const db::mysql::create& request);

        int Query(const db::mysql::query& request, db::mysql::response& response);

	 private:
		bool OnInit() final;
		bool OnStart() final;
        void OnClose() final;
		int GetClientId(int flag = 0);
	private:
		SqlHelper mSqlHelper;
		std::vector<int> mClientIds;
		std::queue<int> mClientIdQueue;
		class ProtoComponent * mProtoComponent;
        class MysqlDBComponent * mMysqlComponent;
        std::unordered_map<std::string, std::string> mMainKeys;
    };
}// namespace Sentry