#include<Component/Component.h>
#include <Pool/ObjectPool.h>
#include"Protocol/s2s.pb.h"
namespace GameKeeper
{
    class MysqlProxyComponent : public Component, public IFrameUpdate, public ILoadData, public INodeProxyRefresh
    {
    public:
        MysqlProxyComponent() = default;

        ~MysqlProxyComponent() final = default;

    protected:
        bool Awake() final;

        void Start() final;

        void OnLodaData() override;

        void OnFrameUpdate(float t) final;

        void OnAddProxyNode(class RpcNodeProxy *node) final;

        void OnDelProxyNode(class RpcNodeProxy *node) final;

    public:
        XCode Add(const Message &data);

        XCode Save(const Message &data);

        XCode Delete(const Message &data);

        XCode Query(const Message &data, Message &queryData);

    private:
        std::queue<unsigned int> mWakeUpQueue;
    private:
        int mMysqlNodeId;
        class CoroutineComponent *mCorComponent;
        class NodeProxyComponent *mNodeComponent;
    };
}