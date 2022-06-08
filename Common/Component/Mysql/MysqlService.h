#pragma once

#include"DB/Mysql/MysqlClient.h"
#include"DB/Mysql/MysqlHelper.h"
#include"Component/RpcService/LocalServiceComponent.h"

namespace Sentry
{
	class MysqlService : public LocalRpcService, public IStart
	{
	 public:
		MysqlService() = default;
		~MysqlService() final = default;
	private:

        XCode Add(const s2s::Mysql::Add& request);

		XCode Save(const s2s::Mysql::Save& request);

		XCode Update(const s2s::Mysql::Update& request);

		XCode Delete(const s2s::Mysql::Delete& request);

        XCode Create(const s2s::Mysql::Create& request);

        XCode Query(const s2s::Mysql::Query& request, s2s::Mysql::Response& response);

	 private:
		bool OnStart() final;
		std::shared_ptr<MysqlClient> GetMysqlClient(long long flag = 0);
		bool OnStartService(ServiceMethodRegister & methodRegister);
	 private:
		size_t mIndex;
		MysqlConfig mConfig;
        class MessageComponent * mMessageComponent;
		std::vector<std::shared_ptr<MysqlClient>> mMysqlClients;
	};
}// namespace Sentry