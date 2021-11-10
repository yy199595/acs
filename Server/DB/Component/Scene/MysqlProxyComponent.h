#include<Component/Component.h>
#include <Pool/ObjectPool.h>
namespace GameKeeper
{
    class MysqlProxyComponent : public Component, public IFrameUpdate, public ILoadData
    {
    public:
        MysqlProxyComponent() {}

        ~MysqlProxyComponent() {}

    protected:
        bool Awake() final;

        void Start() final;

        void OnLodaData() override;

        void OnFrameUpdate(float t) final;

    public:
        XCode Add(const Message &data);

        XCode Save(const Message &data);

        XCode Delete(const Message &data);

        XCode Query(const Message &data, Message &queryData);

    private:
        class NodeProxy *GetServiceNode();

    private:
        std::queue<unsigned int> mWakeUpQueue;
    private:
        int mMysqlProxyNodeId;
        std::string mMessageBuffer;
        class CoroutineComponent *mCorComponent;

        class ServiceNodeComponent *mNodeManager;

        ObjectPool<s2s::MysqlOper_Request> mMysqlOperReqPool;
        ObjectPool<s2s::MysqlOper_Response> mMysqlOperResPool;
        ObjectPool<s2s::MysqlQuery_Request> mMysqlQueryReqPool;
        ObjectPool<s2s::MysqlQuery_Response> mMysqlQueryResPool;
    };
}