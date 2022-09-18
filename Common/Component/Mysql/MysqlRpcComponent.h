//
// Created by zmhy0073 on 2022/7/16.
//

#ifndef SERVER_MYSQLRPCCOMPONENT_H
#define SERVER_MYSQLRPCCOMPONENT_H
#include"DB/Mysql/MysqlDefine.h"
#include"Component/Rpc/RpcTaskComponent.h"

namespace Sentry
{
    class MysqlClient;
    class MysqlRpcComponent : public RpcTaskComponent<std::string>, public IStart
    {
    public:
        MysqlRpcComponent() = default;
        ~MysqlRpcComponent() = default;

    public:

    private:
        bool OnStart() final;
        bool LateAwake() final;

    private:
		MysqlConfig mConfig;
		std::vector<std::shared_ptr<MysqlClient>> mMysqlClients;
    };
}


#endif //SERVER_MYSQLRPCCOMPONENT_H
