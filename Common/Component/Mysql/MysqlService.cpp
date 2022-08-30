#include"MysqlService.h"
#include"App/App.h"
#include"Component/Scene/ProtocolComponent.h"
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
        this->mMessageComponent = this->GetComponent<ProtocolComponent>();

		const ServerConfig & config = this->GetApp()->GetConfig();
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "ip", this->mConfig.mIp));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "port", this->mConfig.mPort));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "user", this->mConfig.mUser));
		LOG_CHECK_RET_FALSE(config.GetMember("mysql", "passwd", this->mConfig.mPassword));

		unsigned int count = std::thread::hardware_concurrency();
		this->GetConfig().GetMember("mysql", "count", count);

		for (unsigned int index = 0; index < count; index++)
		{
			//this->mMysqlClients.emplace_back(std::make_shared<MysqlClient>(this->mConfig));
		}
		return !this->mMysqlClients.empty();
	}

	bool MysqlService::OnStart()
	{
        return  true;
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

    XCode MysqlService::Create(const s2s::mysql::create &request)
    {
		return XCode::Successful;
    }

	XCode MysqlService::Add(const s2s::mysql::add& request)
	{
        return XCode::Successful;
	}

	XCode MysqlService::Save(const s2s::mysql::save& request)
	{
        return XCode::Successful;
    }

	XCode MysqlService::Update(const s2s::mysql::update& request)
	{
        return XCode::Successful;
    }

	XCode MysqlService::Delete(const s2s::mysql::remove& request)
	{
        return XCode::Successful;
    }

	XCode MysqlService::Query(const s2s::mysql::query& request, s2s::mysql::response& response)
	{
        return XCode::Successful;
    }
}// namespace Sentry