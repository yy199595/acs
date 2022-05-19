#pragma once

#include"Protocol/s2s.pb.h"
#include"DB/Mysql/MysqlClient.h"
#include"DB/Mysql/MysqlHelper.h"
#include"Component/RpcService/LocalServiceComponent.h"

namespace Sentry
{
	class MysqlService : public ServiceComponent, public IStart
	{
	 public:
		MysqlService() = default;
		~MysqlService() final = default;
	private:
		XCode Add(const s2s::Mysql::Add& request, s2s::Mysql::Response& response);

		XCode Save(const s2s::Mysql::Save& request, s2s::Mysql::Response& response);

		XCode Update(const s2s::Mysql::Update& request, s2s::Mysql::Response& response);

		XCode Delete(const s2s::Mysql::Delete& request, s2s::Mysql::Response& response);

		XCode Query(const s2s::Mysql::Query& request, s2s::Mysql::Response& response);

		XCode Invoke(const s2s::Mysql::Invoke& request, s2s::Mysql::Response& response);
	 private:
		bool OnStart() final;
		std::shared_ptr<MysqlClient> GetMysqlClient();
		bool OnStartService(ServiceMethodRegister & methodRegister);
	 private:
		MysqlConfig mConfig;
		MysqlHelper mHelper;
		std::vector<std::shared_ptr<MysqlClient>> mMysqlClients;
	};
}// namespace Sentry