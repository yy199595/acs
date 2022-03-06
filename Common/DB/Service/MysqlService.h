#pragma once

#include"Protocol/s2s.pb.h"
#include"Service/RpcService.h"

namespace Sentry
{
    class MysqlService : public RpcService
    {
    public:
        MysqlService() = default;

        ~MysqlService()  final = default;

    public:

        XCode Add(const s2s::Mysql::Add &request, s2s::Mysql::Response &response);

        XCode Save(const s2s::Mysql::Save &request, s2s::Mysql::Response &response);

        XCode Update(const s2s::Mysql::Update & request, s2s::Mysql::Response & response);

        XCode Delete(const s2s::Mysql::Delete &request, s2s::Mysql::Response &response);

        XCode Query(const s2s::Mysql::Query &request, s2s::Mysql::Response &response);

        XCode Invoke(const s2s::Mysql::Invoke & request, s2s::Mysql::Response & response);
    public:
        bool Awake() final;

        bool LateAwake() final;

    private:
        class MysqlComponent *mMysqlComponent;
    };
}// namespace Sentry