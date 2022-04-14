#include"MysqlService.h"
#include"DB/Mysql/MysqlTaskSource.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"App/App.h"
#include"Pool/MessagePool.h"
namespace Sentry
{

	bool MysqlService::OnInitService(ServiceMethodRegister & methodRegister)
	{
		methodRegister.Bind("Add", &MysqlService::Add);
		methodRegister.Bind("Save", &MysqlService::Save);
		methodRegister.Bind("Query", &MysqlService::Query);
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

	XCode MysqlService::Add(const s2s::Mysql::Add& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(request.has_data());
		LOGIC_THROW_ERROR(!request.table().empty());

		std::string sql;
		if (!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
		XCode code = mysqlClient->Invoke(sql, response);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_INFO(sql);
			LOG_ERROR(response.error());
#endif
		}
		return XCode::Successful;
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
		std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
		XCode code = mysqlClient->Invoke(sql, response);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_INFO(sql);
			LOG_ERROR(response.error());
#endif
		}
		return XCode::Successful;
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
		std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
		XCode code = mysqlClient->Invoke(sql, response);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_INFO(sql);
			LOG_ERROR(response.error());
#endif
		}
		return XCode::Successful;
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
		std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
		XCode code = mysqlClient->Invoke(sql, response);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_INFO(sql);
			LOG_ERROR(response.error());
#endif
		}
		return XCode::Successful;
	}

	XCode MysqlService::Invoke(const s2s::Mysql::Invoke& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(!request.sql().empty());
		std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];
		XCode code = mysqlClient->Invoke(request.sql(), response);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_INFO(request.sql());
			LOG_ERROR(response.error());
#endif
		}
		return XCode::Successful;
	}

	XCode MysqlService::Query(const s2s::Mysql::Query& request, s2s::Mysql::Response& response)
	{
		LOGIC_THROW_ERROR(!request.table().empty());
		LOGIC_THROW_ERROR(!request.where_json().empty());
		std::shared_ptr<MysqlClient> mysqlClient = this->mMysqlClients[0];

		std::string sql;
		if(!this->mHelper.ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		XCode code = mysqlClient->Invoke(sql, response);
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_INFO(sql);
			LOG_ERROR(response.error());
#endif
		}
		return XCode::Successful;
	}
}// namespace Sentry