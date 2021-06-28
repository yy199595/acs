#pragma once
#include <Service/LocalService.h>
namespace SoEasy
{
    class MysqlProxy : public LocalService
    {
    public:
        MysqlProxy() {}
        ~MysqlProxy() {}

    public:
        
        XCode Insert(shared_ptr<Message> requestData);
        XCode Update(shared_ptr<Message> requestData);
        XCode Delete(shared_ptr<Message> requestData);
        XCode Query(shared_ptr<Message> requestData, shared_ptr<Message> responseData);
    public:
        bool OnInit() final;

    private:
        class MysqlManager *mMysqlManager;
    }
}