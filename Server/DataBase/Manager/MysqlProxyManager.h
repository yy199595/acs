#include<Manager/Manager.h>
#include<Protocol/s2s.pb.h>
#include<Pool/ObjectPool.h>
namespace SoEasy
{
    class MysqlProxyManager : public Manager, public IFrameUpdate
    {
    public:
        MysqlProxyManager() {}
        ~MysqlProxyManager() {}

    protected:
        bool OnInit() final;
        void OnInitComplete() final;
        void OnFrameUpdate(float t) final;

    public:
        XCode Add(const Message & data);
        XCode Save(const Message & data);
        XCode Delete(const Message & data);
        XCode Query(const Message & data, Message & queryData);
    private:
        class ServiceNode *GetServiceNode();
    private:
        std::queue<long long> mWakeUpQueue;
    private:
        int mMysqlProxyNodeId;
        class CoroutineManager *mCorManager;
        class ServiceNodeManager *mNodeManager;
        ObjectPool<s2s::MysqlOper_Request> mMysqlOperReqPool;
        ObjectPool<s2s::MysqlOper_Response> mMysqlOperResPool;

        ObjectPool<s2s::MysqlQuery_Request> mMysqlQueryReqPool;
        ObjectPool<s2s::MysqlQuery_Response> mMysqlQueryResPool;
    };
}