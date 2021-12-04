#pragma once

#include <Protocol/s2s.pb.h>
#include <Service/ProtoServiceComponent.h>

namespace GameKeeper
{
    class MysqlService : public ProtoServiceComponent
    {
    public:
        MysqlService();

        ~MysqlService()  final = default;

    public:
        XCode Add(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response);

        XCode Save(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response);

        XCode Delete(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response);

        XCode Query(const s2s::MysqlQuery_Request &request, s2s::MysqlQuery_Response &response);

        XCode Invoke(const s2s::MysqlAnyOper_Request & request, s2s::MysqlAnyOper_Response & response);
    public:
        bool Awake() final;

        void Start() final;

    private:

        class MysqlComponent *mMysqlManager;

        class CoroutineComponent *mCorComponent;

        class TaskPoolComponent *mTaskManager;
    };
}// namespace GameKeeper