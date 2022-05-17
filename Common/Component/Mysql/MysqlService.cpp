#include"MysqlService.h"
#include"App/App.h"
#include"Pool/MessagePool.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{

	bool MysqlService::OnStartService(ServiceMethodRegister & methodRegister)
	{
		methodRegister.Bind("Add", &MysqlService::Add);
		methodRegister.Bind("Save", &MysqlService::Save);
		methodRegister.Bind("Query", &MysqlService::Query);
		methodRegister.Bind("Update", &MysqlService::Update);
		methodRegister.Bind("Delete", &MysqlService::Delete);
		methodRegister.Bind("Invoke", &MysqlService::Invoke);

		const ServerConfig & config = this->GetApp()->GetConfig();
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "ip", this->mConfig.mIp));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "port", this->mConfig.mPort));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "user", this->mConfig.mUser));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "passwd", this->mConfig.mPassword));

		unsigned int count = std::thread::hardware_concurrency();
		this->GetApp()->GetConfig().GetMember("mysql", "count", count);

		for (unsigned int index = 0; index < count; index++)
		{
			std::shared_ptr<MysqlClient> mysqlClient(new MysqlClient(this->mConfig));
			this->mMysqlClients.emplace_back(mysqlClient);
		}
		return !this->mMysqlClients.empty();
	}

	bool MysqlService::OnStart()
	{
		std::string address = fmt::format("{0}:{1}", this->mConfig.mIp, this->mConfig.mPort);
		for(std::shared_ptr<MysqlClient> mysqlClient : this->mMysqlClients)
		{
			if(mysqlClient->Start() != 0)
			{
				LOG_ERROR("connect mysql [" << address << "] failure");
				return false;
			}
			LOG_INFO("connect mysql [" << address << "] successful");
		}
		return this->mMysqlClients[0]->InitTable("db.proto") == XCode::Successful;
	}

	std::shared_ptr<MysqlClient> MysqlService::GetMysqlClient()
	{
		return this->mMysqlClients[0];
	}

	XCode MysqlService::Add(const s2s::Mysql::Add& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(request.has_data());
		LOGIC_THROW_ERROR(!request.table().empty());

		std::string sql;
		if (!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		return this->GetMysqlClient()->Invoke(sql, response);
	}

	XCode MysqlService::Save(const s2s::Mysql::Save& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(request.has_data());
		LOGIC_THROW_ERROR(!request.table().empty());

		std::string sql;
		if (!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		return this->GetMysqlClient()->Invoke(sql, response);
	}

	XCode MysqlService::Update(const s2s::Mysql::Update& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(!request.table().empty());
		LOGIC_THROW_ERROR(!request.where_json().empty());

		std::string sql;
		if (!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		return this->GetMysqlClient()->Invoke(sql, response);
	}

	XCode MysqlService::Delete(const s2s::Mysql::Delete& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(!request.table().empty());
		LOGIC_THROW_ERROR(!request.where_json().empty());

		std::string sql;
		if (!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		return this->GetMysqlClient()->Invoke(sql, response);
	}

	XCode MysqlService::Invoke(const s2s::Mysql::Invoke& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(!request.sql().empty());
		return this->GetMysqlClient()->Invoke(request.sql(), response);
	}

	XCode MysqlService::Query(const s2s::Mysql::Query& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(!request.table().empty());
		LOGIC_THROW_ERROR(!request.where_json().empty());

		std::string sql;
		if(!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		return this->GetMysqlClient()->Invoke(sql, response);
	}
}// namespace Sentry