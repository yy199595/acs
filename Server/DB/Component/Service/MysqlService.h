#pragma once

#include <Protocol/s2s.pb.h>
#include <Service/LocalServiceComponent.h>

namespace Sentry
{
    class MysqlService : public LocalServiceComponent
    {
    public:
        MysqlService();

        ~MysqlService() {}

    public:
        XCode Add(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response);

        XCode Save(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response);

        XCode Delete(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response);

        XCode QueryData(const s2s::MysqlQuery_Request &request, s2s::MysqlQuery_Response &response);

    public:
        bool Awake() final;

        void Start() final;

    private:

        class MysqlComponent *mMysqlManager;

        class CoroutineComponent *mCorComponent;

        class TaskComponent *mTaskManager;

        class ProtocolComponent * mProtocolManager;
    };
}// namespace Sentry