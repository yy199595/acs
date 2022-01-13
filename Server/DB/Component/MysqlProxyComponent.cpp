#include "MysqlProxyComponent.h"

#include"Core/App.h"
#include"Service/ServiceEntity.h"
#include"Pool/MessagePool.h"
#include"Util/StringHelper.h"
#include"Util/MathHelper.h"
#include"Rpc/RpcComponent.h"
#include"Scene/ServiceComponent.h"
#include"Other/ElapsedTimer.h"
#include"Other/StringFmt.h"
#include"MysqlClient/MysqlRpcTaskSource.h"
namespace GameKeeper
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
        this->mServiceComponent = this->GetComponent<ServiceComponent>();
        return true;
    }


    void MysqlProxyComponent::OnLoadData()
    {
        this->AddUserData();
//		ElapsedTimer timer;
//		auto group = this->mCorComponent->NewCoroutineGroup();
//		for (int index = 0; index < 2; index++)
//		{
//			group->Add(this->mCorComponent->Start(&MysqlProxyComponent::AddUserData, this));
//			group->Add(this->mCorComponent->Start(&MysqlProxyComponent::SortUserData, this));
//		}
//		group->AwaitAll();
//		LOG_ERROR("use time = " << timer.GetSecond() << "s");
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


            std::shared_ptr<MysqlRpcTaskSource> taskSource(new MysqlRpcTaskSource());
            if(this->Add(userAccountData, taskSource) == XCode::Successful)
            {
                LOG_ERROR("add data successful {0}", index);
            }
        }
        db::db_account_tab_user_account userAccount;
        userAccount.set_account("10000@qq.com");
        std::shared_ptr<MysqlRpcTaskSource> taskSource(new MysqlRpcTaskSource());

       if(this->Query(userAccount, taskSource) == XCode::Successful)
       {

       }

        std::shared_ptr<MysqlRpcTaskSource> rpcTaskSource(new MysqlRpcTaskSource());
        XCode code = this->Sort("db_account.tab_user_account", "user_id", 10, false, rpcTaskSource);
        if(code == XCode::Successful)
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
        auto mysqlEntity = this->mServiceComponent->GetServiceEntity("MysqlService");
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
        std::string address;
        if(!mysqlEntity->AllotServiceAddress(address))
        {
            LOG_ERROR("allot MysqlService address failure", address);
            return nullptr;
        }
        std::shared_ptr<ServiceNode> serviceNode = mysqlEntity->GetNode(address);
        if(serviceNode == nullptr)
        {
            LOG_ERROR("not find node : ", address);
            return nullptr;
        }
        serviceNode->PushMessage(requestMessage);
        return requestMessage;
    }

    XCode MysqlProxyComponent::Add(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        auto requestMessage = this->NewMessage("Add");
        if(requestMessage == nullptr)
        {
            return XCode::Failure;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpc_id(taskSource->GetRpcId());
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Query(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        auto requestMessage = this->NewMessage("Query");
        if(requestMessage == nullptr)
        {
            return XCode::Failure;
        }

        this->mQueryRequest.Clear();
        this->mQueryRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mQueryRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        auto requestMessage = this->NewMessage("Invoke");
        if(requestMessage == nullptr)
        {
            return XCode::Failure;
        }

        this->mAnyOperRequest.Clear();
        this->mAnyOperRequest.set_sql(sql);
        this->mAnyOperRequest.set_tab(tab);
        requestMessage->mutable_data()->PackFrom(this->mAnyOperRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Save(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        auto requestMessage = this->NewMessage("Save");
        if(requestMessage == nullptr)
        {
            return XCode::Failure;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Delete(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        auto requestMessage = this->NewMessage("Delete");
        if(requestMessage == nullptr)
        {
            return XCode::Failure;
        }
        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        if(taskSource!= nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Sort(const std::string &tab, const std::string &field, int count, bool reverse, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        const char * type = !reverse ? "ASC" : "DESC";
        return this->Invoke(tab, fmt::format(
                "select * from {0} ORDER BY {1} {2} LIMIT {3}", tab, field, type, count), taskSource);
    }
}