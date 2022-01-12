#include"Component/Component.h"
#include "Pool/ObjectPool.h"
#include"Protocol/s2s.pb.h"
namespace GameKeeper
{
    class NodeHelper;
    class MysqlRpcTaskSource;
    class MysqlProxyComponent : public Component, public ILoadData, public INodeRefresh
    {
    public:
        MysqlProxyComponent() = default;

        ~MysqlProxyComponent() final = default;

    protected:
        bool Awake() final;

        bool LateAwake() final;

        void OnLoadData() override;

        void OnAddRpcNode(class RpcNode *node) final;

        void OnDelRpcNode(class RpcNode *node) final;
	private:
		void AddUserData();
    public:
        XCode Add(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource = nullptr);

        XCode Save(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource = nullptr);

        XCode Delete(const Message &data,std::shared_ptr<MysqlRpcTaskSource> taskSource = nullptr);

        XCode Query(const Message &data,std::shared_ptr<MysqlRpcTaskSource> taskSource = nullptr);

        XCode Invoke(const std::string & tab, const std::string & sql,std::shared_ptr<MysqlRpcTaskSource> taskSource = nullptr);

        XCode Sort(const std::string & tab, const std::string & field, int count, bool reverse = false,std::shared_ptr<MysqlRpcTaskSource> taskSource = nullptr);

    private:
        RpcNode * GetMysqlNode();
    private:
        std::vector<int> mMysqlServices;
        class RpcComponent * mRpcComponent;
        class TaskComponent *mCorComponent;
        class RpcNodeComponent *mNodeComponent;
        s2s::MysqlOper_Request mOperRequest;
        s2s::MysqlQuery_Request mQueryRequest;
        s2s::MysqlAnyOper_Request mAnyOperRequest;
    };
}