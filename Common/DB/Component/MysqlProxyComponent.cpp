#include "MysqlProxyComponent.h"

#include"Object/App.h"
#include"Service/ServiceProxy.h"
#include"Pool/MessagePool.h"
#include"Util/StringHelper.h"
#include"Rpc/RpcComponent.h"
#include"Scene/ServiceMgrComponent.h"
#include"Other/ElapsedTimer.h"
#include"Other/StringFmt.h"
#include"DB/MysqlClient/MysqlRpcTaskSource.h"
namespace Sentry
{
    bool MysqlProxyComponent::Awake()
    {
		this->mCorComponent = nullptr;
        return true;
    }

    bool MysqlProxyComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mRpcComponent = this->GetComponent<RpcComponent>();
        this->mServiceComponent = this->GetComponent<ServiceMgrComponent>();
        return true;
    }

    void MysqlProxyComponent::OnComplete()
    {
        auto taskComponent = this->GetComponent<TaskComponent>();
        //taskComponent->Start(&MysqlProxyComponent::AddUserData, this);
    }

	void MysqlProxyComponent::AddUserData()
	{
		for (int index = 0; index < 10; index++)
        {
            db::db_account::tab_user_account userAccountData;
            userAccountData.set_user_id(index + 10000);
            userAccountData.set_account(std::to_string(index + 10000) + "@qq.com");
            userAccountData.set_device_mac("ios_qq");
            userAccountData.set_token(Helper::String::CreateNewToken());
            userAccountData.set_register_time(Helper::Time::GetSecTimeStamp());


            std::shared_ptr<MysqlRpcTaskSource> taskSource =  this->Add(userAccountData);
            if(taskSource != nullptr && taskSource->GetCode() == XCode::Successful)
            {
                LOG_ERROR("add data successful ", index);
            }
        }
        db::db_account_tab_user_account userAccount;
        userAccount.set_account("10000@qq.com");
        std::shared_ptr<MysqlRpcTaskSource> taskSource = this->Query(userAccount);
        if(taskSource != nullptr && taskSource->GetCode() == XCode::Successful)
        {

        }

        std::shared_ptr<MysqlRpcTaskSource> rpcTaskSource =this->Sort("db_account.tab_user_account", "user_id", 10, false);
        if(rpcTaskSource != nullptr && rpcTaskSource->GetCode() == XCode::Successful)
        {
            size_t size = rpcTaskSource->GetDataSize();
            for(size_t index = 0; index < size; index++)
            {
                auto data = rpcTaskSource->GetData<db::db_account_tab_user_account>(index);
                LOG_WARN("json = ", Helper::Proto::ToJson(*data));
            }
        }
	}

    std::shared_ptr<com::Rpc_Request> MysqlProxyComponent::NewMessage(const std::string &name)
    {
        auto mysqlEntity = this->mServiceComponent->GetServiceProxy("MysqlService");
        if(mysqlEntity == nullptr)
        {
            return nullptr;
        }
        std::string func = fmt::format("MysqlService.{0}", name);
        auto requestMessage = mysqlEntity->NewRequest(func);
        if(requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method ", func);
            return nullptr;
        }
        std::string address = mysqlEntity->AllotAddress();
        if(address.empty())
        {
            LOG_ERROR("allot MysqlService address failure", address);
            return nullptr;
        }
        std::shared_ptr<ProxyClient> serviceNode = mysqlEntity->GetNode(address);
        if(serviceNode == nullptr)
        {
            LOG_ERROR("not find node : ", address);
            return nullptr;
        }
        serviceNode->PushMessage(requestMessage);
        return requestMessage;
    }

    std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Add(const Message &data)
    {
        auto requestMessage = this->NewMessage("Add");
        if (requestMessage == nullptr) {
            return nullptr;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        std::shared_ptr<MysqlRpcTaskSource> mysqlRpcTaskSource(new MysqlRpcTaskSource());


        this->mRpcComponent->AddRpcTask(mysqlRpcTaskSource);
        requestMessage->set_rpc_id(mysqlRpcTaskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(mysqlRpcTaskSource->GetRpcId(), requestMessage->method_id());
#endif
        return mysqlRpcTaskSource;
    }

    std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Query(const Message &data)
    {
        auto requestMessage = this->NewMessage("Query");
        if (requestMessage == nullptr) {
            return nullptr;
        }

        this->mQueryRequest.Clear();
        this->mQueryRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mQueryRequest);
        std::shared_ptr<MysqlRpcTaskSource> mysqlRpcTaskSource(new MysqlRpcTaskSource());

        this->mRpcComponent->AddRpcTask(mysqlRpcTaskSource);
        requestMessage->set_rpc_id(mysqlRpcTaskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(mysqlRpcTaskSource->GetRpcId(), requestMessage->method_id());
#endif
        return mysqlRpcTaskSource;
    }

    std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql)
    {
        auto requestMessage = this->NewMessage("Invoke");
        if (requestMessage == nullptr) {
            return nullptr;
        }

        this->mAnyOperRequest.Clear();
        this->mAnyOperRequest.set_sql(sql);
        this->mAnyOperRequest.set_tab(tab);
        requestMessage->mutable_data()->PackFrom(this->mAnyOperRequest);
        std::shared_ptr<MysqlRpcTaskSource> mysqlRpcTaskSource(new MysqlRpcTaskSource());

        this->mRpcComponent->AddRpcTask(mysqlRpcTaskSource);
        requestMessage->set_rpc_id(mysqlRpcTaskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(mysqlRpcTaskSource->GetRpcId(), requestMessage->method_id());
#endif
        return mysqlRpcTaskSource;
    }

    std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Save(const Message &data)
    {
        auto requestMessage = this->NewMessage("Save");
        if (requestMessage == nullptr) {
            return nullptr;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        std::shared_ptr<MysqlRpcTaskSource> mysqlRpcTaskSource(new MysqlRpcTaskSource());

        this->mRpcComponent->AddRpcTask(mysqlRpcTaskSource);
        requestMessage->set_rpc_id(mysqlRpcTaskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(mysqlRpcTaskSource->GetRpcId(), requestMessage->method_id());
#endif
        return mysqlRpcTaskSource;
    }

    std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Delete(const Message &data)
    {
        auto requestMessage = this->NewMessage("Delete");
        if (requestMessage == nullptr) {
            return nullptr;
        }
        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        std::shared_ptr<MysqlRpcTaskSource> mysqlRpcTaskSource(new MysqlRpcTaskSource());


        this->mRpcComponent->AddRpcTask(mysqlRpcTaskSource);
        requestMessage->set_rpc_id(mysqlRpcTaskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(mysqlRpcTaskSource->GetRpcId(), requestMessage->method_id());
#endif
        return mysqlRpcTaskSource;
    }

    std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Sort(const std::string &tab, const std::string &field, int count, bool reverse)
    {
        const char * type = !reverse ? "ASC" : "DESC";
        return this->Invoke(tab, fmt::format(
                "select * from {0} ORDER BY {1} {2} LIMIT {3}", tab, field, type, count));
    }
}