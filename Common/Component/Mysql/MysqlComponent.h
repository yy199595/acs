//
// Created by zmhy0073 on 2022/7/16.
//

#ifndef SERVER_MYSQLCOMPONENT_H
#define SERVER_MYSQLCOMPONENT_H
#include"DB/Mysql/MysqlDefine.h"
#include"Component/Rpc/RpcTaskComponent.h"

namespace Sentry
{
    class MysqlClient;
    class MysqlComponent : public RpcTaskComponent<std::string>, public IStart
    {
    public:
        MysqlComponent() = default;
        ~MysqlComponent() = default;

    public:

    private:
        bool OnStart() final;

    private:
        MysqlConfig mConfig;
    };
}


#endif //SERVER_MYSQLCOMPONENT_H
