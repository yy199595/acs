//
// Created by zmhy0073 on 2022/7/16.
//

#include"MysqlRpcComponent.h"
#include"DB/Mysql/MysqlClient.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
    bool MysqlRpcComponent::OnStart()
    {
#ifdef ONLY_MAIN_THREAD
        IAsioThread & t = this->GetApp()->GetTaskScheduler();
#else
        NetThreadComponent * component = this->GetComponent<NetThreadComponent>();
        IAsioThread & t = component->AllocateNetThread();
#endif
        std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(t, "114.115.167.51", 3306);
        MysqlClient mysqlClient(socketProxy, this->mConfig, this);

        return true;
    }
}