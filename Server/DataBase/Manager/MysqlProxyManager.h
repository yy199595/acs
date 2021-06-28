#include <Manager/Manager.h>

namespace SoEasy
{
    class MysqlProxyManager : public Manager
    {
    public:
        MysqlProxyManager() {}
        ~MysqlProxyManager() {}

    protected:
        bool OnInit() final;
        void OnInitComplete() final;
        void OnSystemUpdate() final;

    public:
        XCode Add(shared_ptr<Message> data);
        XCode Query(shared_ptr<Message> data);
        XCode Update(shared_ptr<Message> data);
        XCode Delete(shared_ptr<Message> data);
    private:
        class ServiceNode *GetServiceNode();

    private:
        int mMysqlProxyNodeId;
        class CoroutineManager *mCorManager;
        class ServiceNodeManager *mNodeManager;
    };
}