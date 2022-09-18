//
// Created by zmhy0073 on 2022/7/16.
//

#include"MysqlRpcComponent.h"
#include"DB/Mysql/MysqlClient.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
    bool MysqlRpcComponent::LateAwake()
	{
		const ServerConfig& config = this->GetConfig();
		config.GetMember("mysql", "ip", this->mConfig.mIp);
		config.GetMember("mysql", "port", this->mConfig.mPort);
		config.GetMember("mysql", "user", this->mConfig.mUser);
		config.GetMember("mysql", "count", this->mConfig.mMaxCount);
		config.GetMember("mysql", "passwd", this->mConfig.mPassword);
		return true;
	}

    bool MysqlRpcComponent::OnStart()
	{
		for (int index = 0; index < this->mConfig.mMaxCount; index++)
		{
			std::shared_ptr<MysqlClient> mysqlClient
				= std::make_shared<MysqlClient>(this->mConfig, this);
			if (!mysqlClient->StartConnect())
			{
				return false;
			}
			this->mMysqlClients.emplace_back(mysqlClient);
		}
		return true;
	}
}