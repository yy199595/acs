#pragma once

#include"DB/Mysql/MysqlClient.h"
#include"DB/Mysql/MysqlHelper.h"
#include"Component/RpcService/LocalService.h"

namespace Sentry
{
	class MysqlService : public LocalService, public IStart
	{
	 public:
		MysqlService() = default;
		~MysqlService() final = default;
	private:

        XCode Add(const s2s::mysql::add& request);

		XCode Save(const s2s::mysql::save& request);

		XCode Update(const s2s::mysql::update& request);

		XCode Delete(const s2s::mysql::remove& request);

        XCode Create(const s2s::mysql::create& request);

        XCode Query(const s2s::mysql::query& request, s2s::mysql::response& response);

	 private:
		bool OnStart() final;
		std::shared_ptr<MysqlClient> GetMysqlClient(long long flag = 0);
		bool OnStartService(ServiceMethodRegister & methodRegister);
	 private:
		size_t mIndex;
		MysqlConfig mConfig;
        class ProtocolComponent * mMessageComponent;
		std::vector<std::shared_ptr<MysqlClient>> mMysqlClients;
	};
}// namespace Sentry