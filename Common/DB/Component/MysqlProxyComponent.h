#include"Component/Component.h"
#include "Pool/ObjectPool.h"
#include"Protocol/s2s.pb.h"
namespace Sentry
{
    class ServiceProxy;
    class MysqlRpcTaskSource;
    class MysqlProxyComponent : public Component
    {
    public:
        MysqlProxyComponent() = default;

        ~MysqlProxyComponent() final = default;

    protected:
        bool Awake() final;

        bool LateAwake() final;

        void OnComplete() final;
	private:
		void AddUserData();
        std::shared_ptr<com::Rpc_Request> NewMessage(const std::string & name);
    public:
        std::shared_ptr<MysqlRpcTaskSource> Add(const Message &data);

        std::shared_ptr<MysqlRpcTaskSource> Save(const Message &data);

        std::shared_ptr<MysqlRpcTaskSource> Delete(const Message &data);

        std::shared_ptr<MysqlRpcTaskSource> Query(const Message &data);

        std::shared_ptr<MysqlRpcTaskSource> Invoke(const std::string & tab, const std::string & sql);

        std::shared_ptr<MysqlRpcTaskSource> Sort(const std::string & tab, const std::string & field, int count, bool reverse = false);

    private:
        class RpcComponent * mRpcComponent;
        class TaskComponent *mCorComponent;
        s2s::MysqlOper_Request mOperRequest;
        s2s::MysqlQuery_Request mQueryRequest;
        s2s::MysqlAnyOper_Request mAnyOperRequest;
        class ServiceMgrComponent * mServiceComponent;
    };
}