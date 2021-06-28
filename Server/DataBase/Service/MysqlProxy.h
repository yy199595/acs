#pragma once
#include <Service/LocalService.h>
namespace SoEasy
{
    class MysqlProxy : public LocalService
    {
    public:
        MysqlProxy();
        ~MysqlProxy() {}
    public:     
        XCode Insert( Message & requestData);
        XCode Update( Message & requestData);
        XCode Delete( Message & requestData);
        XCode Query( Message & requestData, Message & responseData);
    public:
        bool OnInit() final;
    private:
        int mMysqlNodeId;
        class MysqlManager *mMysqlManager;
        class CoroutineManager * mCorManager;
    };
}