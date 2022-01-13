#pragma once

#include"Protocol/s2s.pb.h"
#include"Component/ServiceBase/ServiceComponentBase.h"

namespace GameKeeper
{
    class MysqlService : public ServiceComponentBase
    {
    public:
        MysqlService() = default;

        ~MysqlService()  final = default;

    public:
        XCode Add(const s2s::MysqlOper_Request &request, s2s::MysqlResponse &response);

        XCode Save(const s2s::MysqlOper_Request &request, s2s::MysqlResponse &response);

        XCode Delete(const s2s::MysqlOper_Request &request, s2s::MysqlResponse &response);

        XCode Query(const s2s::MysqlQuery_Request &request, s2s::MysqlResponse &response);

        XCode Invoke(const s2s::MysqlAnyOper_Request & request, s2s::MysqlResponse & response);
    public:
        bool Awake() final;

        bool LateAwake() final;

    private:
        class MysqlComponent *mMysqlComponent;
    };
}// namespace GameKeeper