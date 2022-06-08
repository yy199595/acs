#include"MysqlService.h"
#include"App/App.h"
#include"Component/Scene/MessageComponent.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{

	bool MysqlService::OnStartService(ServiceMethodRegister & methodRegister)
	{
		this->mIndex = 0;
		methodRegister.Bind("Add", &MysqlService::Add);
		methodRegister.Bind("Save", &MysqlService::Save);
		methodRegister.Bind("Query", &MysqlService::Query);
		methodRegister.Bind("Update", &MysqlService::Update);
		methodRegister.Bind("Delete", &MysqlService::Delete);
        this->mMessageComponent = this->GetComponent<MessageComponent>();

		const ServerConfig & config = this->GetApp()->GetConfig();
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "ip", this->mConfig.mIp));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "port", this->mConfig.mPort));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "user", this->mConfig.mUser));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "passwd", this->mConfig.mPassword));

		unsigned int count = std::thread::hardware_concurrency();
		this->GetConfig().GetMember("mysql", "count", count);

		for (unsigned int index = 0; index < count; index++)
		{
			this->mMysqlClients.emplace_back(std::make_shared<MysqlClient>(this->mConfig));
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
		std::shared_ptr<MysqlTableTaskSource> tableTaskSource
			= std::make_shared<MysqlTableTaskSource>("db.proto");
		return this->GetMysqlClient()->Start(tableTaskSource) == XCode::Successful;
	}

	std::shared_ptr<MysqlClient> MysqlService::GetMysqlClient(long long flag)
	{
		if(flag == 0)
		{
			if(this->mIndex >= this->mMysqlClients.size())
			{
				this->mIndex = 0;
			}
			return this->mMysqlClients[this->mIndex++];
		}
		size_t index = flag % this->mMysqlClients.size();
		return this->mMysqlClients[index];
	}

    XCode MysqlService::Create(const s2s::Mysql::Create &request)
    {

    }

	XCode MysqlService::Add(const s2s::Mysql::Add& request)
	{
		std::shared_ptr<Mysql::MysqlAddCommandTask> commandTask
				= std::make_shared<Mysql::MysqlAddCommandTask>(request);
		return this->GetMysqlClient()->Start(commandTask);
	}

	XCode MysqlService::Save(const s2s::Mysql::Save& request)
	{
		std::shared_ptr<Mysql::MysqlSaveCommandTask> commandTask
				= std::make_shared<Mysql::MysqlSaveCommandTask>(request);
		return this->GetMysqlClient()->Start(commandTask);
	}

	XCode MysqlService::Update(const s2s::Mysql::Update& request)
	{
		std::shared_ptr<Mysql::MysqlUpdateCommandTask> commandTask
				= std::make_shared<Mysql::MysqlUpdateCommandTask>(request);
		return this->GetMysqlClient()->Start(commandTask);
	}

	XCode MysqlService::Delete(const s2s::Mysql::Delete& request)
	{
		std::shared_ptr<Mysql::MysqlDeleteCommandTask> commandTask
				= std::make_shared<Mysql::MysqlDeleteCommandTask>(request);
		return this->GetMysqlClient()->Start(commandTask);
	}

	XCode MysqlService::Query(const s2s::Mysql::Query& request, s2s::Mysql::Response& response)
	{
        std::shared_ptr<Message> message = this->mMessageComponent->New(request.table());
        if(message == nullptr)
        {
            LOG_ERROR(request.table() << " not exist");
            return XCode::CallArgsError;
        }

		std::shared_ptr<Mysql::MysqlQueryCommandTask> commandTask
				= std::make_shared<Mysql::MysqlQueryCommandTask>(request, response, *message);
		return this->GetMysqlClient()->Start(commandTask);
	}
}// namespace Sentry