#pragma once

#include"Client/MysqlClient.h"
#include"Client/MysqlHelper.h"
#include"Service/LocalRpcService.h"

namespace Sentry
{
	class MysqlService : public LocalRpcService
	{
	 public:
		MysqlService() = default;
	private:

        XCode Add(const db::mysql::add& request);

		XCode Save(const db::mysql::save& request);

		XCode Update(const db::mysql::update& request);

		XCode Delete(const db::mysql::remove& request);

        XCode Create(const db::mysql::create& request);

        XCode Query(const db::mysql::query& request, db::mysql::response& response);

	 private:
        bool Awake();
		bool OnStart() final;
        bool OnClose() final;
	 private:
        class ProtoComponent * mProtoComponent;
        class MysqlDBComponent * mMysqlComponent;
        class DataSyncComponent * mSyncComponent;
        std::shared_ptr<MysqlHelper> mMysqlHelper;
        std::unordered_map<std::string, std::string> mMainKeys;
    };
}// namespace Sentry