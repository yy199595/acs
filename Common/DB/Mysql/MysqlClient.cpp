//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Component/Mysql/MysqlRpcComponent.h"
#include"MysqlMessage.h"
#include"Util/sha1.h"
namespace Sentry
{
	MysqlClient::MysqlClient(MysqlConfig &config, MysqlRpcComponent *component)
							 :std::thread(std::bind(&MysqlClient::Update, this)), mConfig(config)
	{
		this->mMysqlClient = nullptr;
        this->mComponent = component;
	}

	void MysqlClient::Update()
	{

	}

	bool MysqlClient::StartConnect()
	{
		std::lock_guard<std::mutex> lock(this->mLock);

		mysql_init(this->mMysqlClient);
		const char* ip = this->mConfig.mIp.c_str();
		const char* user = this->mConfig.mUser.c_str();
		const char* pwd = this->mConfig.mPassword.c_str();
		const unsigned short port = this->mConfig.mPort;
		if (!mysql_real_connect(this->mMysqlClient, ip, user, pwd, NULL, port, NULL, 0))
		{
			CONSOLE_LOG_ERROR("connect mysql [" << this->mConfig.mAddress << "] failure");
			return false;
		}
		CONSOLE_LOG_INFO("connect mysql [" << this->mConfig.mAddress << "] successful");
		return true;
	}
}
