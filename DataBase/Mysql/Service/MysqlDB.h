#pragma once

#include"Client/MysqlClient.h"
#include"Client/MysqlHelper.h"
#include"Service/PhysicalService.h"

namespace Sentry
{
	class MysqlDB : public PhysicalService
	{
	 public:
		MysqlDB() = default;
	private:

        int Add(const db::mysql::add& request);

		int Save(const db::mysql::save& request);

		int Update(const db::mysql::update& request);

		int Delete(const db::mysql::remove& request);

        int Create(const db::mysql::create& request);

        int Query(const db::mysql::query& request, db::mysql::response& response);

	 private:
        bool Awake();
		bool OnStart() final;
        bool OnClose() final;
	 private:
        class ProtoComponent * mProtoComponent;
        class MysqlDBComponent * mMysqlComponent;
        std::shared_ptr<MysqlHelper> mMysqlHelper;
        std::unordered_map<std::string, std::string> mMainKeys;
    };
}// namespace Sentry