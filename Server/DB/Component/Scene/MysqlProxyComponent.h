#include<Component/Component.h>
#include <Pool/ObjectPool.h>
#include"Protocol/s2s.pb.h"
namespace GameKeeper
{
    class MysqlRpcTask;
    class MysqlProxyComponent : public Component, public IFrameUpdate, public ILoadData, public INodeProxyRefresh
    {
    public:
        MysqlProxyComponent() = default;

        ~MysqlProxyComponent() final = default;

    protected:
        bool Awake() final;

        bool LateAwake() final;

        void OnLoadData() override;

        void OnFrameUpdate(float t) final;

        void OnAddProxyNode(class RpcNodeProxy *node) final;

        void OnDelProxyNode(class RpcNodeProxy *node) final;
	private:
		void AddUserData();
		void SortUserData();
    public:
        std::shared_ptr<MysqlRpcTask> Add(const Message &data);

        std::shared_ptr<MysqlRpcTask> Save(const Message &data);

        std::shared_ptr<MysqlRpcTask> Delete(const Message &data);

        std::shared_ptr<MysqlRpcTask> Query(const Message &data);

        std::shared_ptr<MysqlRpcTask> Invoke(const std::string & tab, const std::string & sql);

        std::shared_ptr<MysqlRpcTask> Sort(const std::string & tab, const std::string & field, int count, bool reverse = false);

    private:
        int mMysqlNodeId;
        s2s::MysqlOper_Request mOperRequest;
        s2s::MysqlQuery_Request mQueryRequest;
        s2s::MysqlAnyOper_Request mAnyOperRequest;
        class TaskComponent *mCorComponent;
        class NodeProxyComponent *mNodeComponent;
    };
}