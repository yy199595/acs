#include<Component/Component.h>
#include <Pool/ObjectPool.h>
namespace Sentry
{
    class SceneMysqlProxyComponent : public Component, public IFrameUpdate
    {
    public:
        SceneMysqlProxyComponent() {}

        ~SceneMysqlProxyComponent() {}

    protected:
        bool Awake() final;

        void Start() final;

        void OnFrameUpdate(float t) final;

    public:
        XCode Add(const Message &data);

        XCode Save(const Message &data);

        XCode Delete(const Message &data);

        XCode Query(const Message &data, Message &queryData);

    private:
        class ServiceNode *GetServiceNode();

    private:
        std::queue<long long> mWakeUpQueue;
    private:
        int mMysqlProxyNodeId;

        class CoroutineComponent *mCorComponent;

        class ServiceNodeComponent *mNodeManager;

        ObjectPool<s2s::MysqlOper_Request> mMysqlOperReqPool;
        ObjectPool<s2s::MysqlOper_Response> mMysqlOperResPool;
        ObjectPool<s2s::MysqlQuery_Request> mMysqlQueryReqPool;
        ObjectPool<s2s::MysqlQuery_Response> mMysqlQueryResPool;
    };
}