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
        const ServerConfig &config = this->GetConfig();
        config.GetMember("mysql", "ip", this->mConfig.mIp);
        config.GetMember("mysql", "port", this->mConfig.mPort);
        config.GetMember("mysql", "user", this->mConfig.mUser);
        config.GetMember("mysql", "passwd", this->mConfig.mPassword);

        return true;
    }

    bool MysqlRpcComponent::OnStart()
    {
        NetThreadComponent * threadComponent = this->GetComponent<NetThreadComponent>();
        std::shared_ptr<SocketProxy> socketProxy = threadComponent->CreateSocket(this->mConfig.mIp, this->mConfig.mPort);

        std::shared_ptr<MysqlClient> mysqlClient(new MysqlClient(socketProxy, this->mConfig, this));
        return true;
    }
}