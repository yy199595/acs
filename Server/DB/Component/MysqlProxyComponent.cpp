#include "MysqlProxyComponent.h"

#include"Core/App.h"
#include"Service/RpcNode.h"
#include"Pool/MessagePool.h"
#include"Util/StringHelper.h"
#include"Util/MathHelper.h"
#include"Rpc/RpcComponent.h"
#include"Service/RpcNodeComponent.h"
#include"Other/ElapsedTimer.h"
#include"MysqlClient/MysqlRpcTaskSource.h"
namespace GameKeeper
{
    bool MysqlProxyComponent::Awake()
    {
        this->mNodeId = -1;
		this->mCorComponent = nullptr;
        return true;
    }

    bool MysqlProxyComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mRpcComponent = this->GetComponent<RpcComponent>();
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<RpcNodeComponent>());
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

    void MysqlProxyComponent::OnAddRpcNode(RpcNode *node)
    {
        if(node->HasService("MysqlService"))
        {
            this->mNodeId = node->GetGlobalId();
        }
    }

    void MysqlProxyComponent::OnDelRpcNode(RpcNode *node)
    {
        if(this->mNodeId == node->GetGlobalId())
        {
            this->mNodeId = -1;
        }
    }

	void MysqlProxyComponent::AddUserData()
	{
		long long userId = Helper::Math::Random(10000, 10000000);
		for (int index = 0; index < 10; index++)
        {
            int random = Helper::Math::Random<int>(100, 100000);
            db::UserAccountData userAccountData;
            userAccountData.set_userid(userId + random);
            userAccountData.set_account(std::to_string(userId + index) + "@qq.com");
            userAccountData.set_devicemac("ios_qq");
            userAccountData.set_token(Helper::String::CreateNewToken());
            userAccountData.set_registertime(Helper::Time::GetSecTimeStamp());

            std::shared_ptr<MysqlRpcTaskSource> taskSource(new MysqlRpcTaskSource());
            if(this->Add(userAccountData, taskSource) == XCode::Successful)
            {
                LOG_ERROR("add data successful " << index);
            }
        }

        std::shared_ptr<MysqlRpcTaskSource> rpcTaskSource(new MysqlRpcTaskSource());
        XCode code = this->Sort("tb_player_account", "UserID", 10, false, rpcTaskSource);
        if(code == XCode::Successful)
        {
            size_t size = rpcTaskSource->GetDataSize();
            for(size_t index = 0; index < size; index++)
            {
                std::string json;
                auto data = rpcTaskSource->GetData<db::UserAccountData>(index);
                Helper::Proto::GetJson(*data, json);
                LOG_WARN(json);
            }
        }
	}

    XCode MysqlProxyComponent::Add(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        RpcNode *proxyNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if (proxyNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mNodeId);
            return XCode::CallServiceNotFound;
        }

        auto requestMessage = proxyNode->NewRequest("MysqlService.Add");
        if(requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Add");
            return XCode::NotFoundRpcConfig;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpcid(taskSource->GetRpcId());
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Query(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mNodeId);
            return XCode::CallServiceNotFound;
        }
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Query");
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Query");
            return XCode::NotFoundRpcConfig;
        }

        this->mQueryRequest.Clear();
        this->mQueryRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mQueryRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpcid(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->methodid());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mNodeId);
            return XCode::CallServiceNotFound;
        }
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Invoke");
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Invoke");
            return XCode::NotFoundRpcConfig;
        }

        this->mAnyOperRequest.Clear();
        this->mAnyOperRequest.set_sql(sql);
        this->mAnyOperRequest.set_tab(tab);
        requestMessage->mutable_data()->PackFrom(this->mAnyOperRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpcid(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->methodid());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Save(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mNodeId);
            return XCode::CallServiceNotFound;
        }
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Save");
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Save");
            return XCode::NotFoundRpcConfig;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpcid(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->methodid());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Delete(const Message &data, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mNodeId);
            return XCode::CallServiceNotFound;
        }
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Delete");
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Delete");
            return XCode::NotFoundRpcConfig;
        }

        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        if(taskSource!= nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestMessage->set_rpcid(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestMessage->methodid());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode MysqlProxyComponent::Sort(const std::string &tab, const std::string &field, int count, bool reverse, std::shared_ptr<MysqlRpcTaskSource> taskSource)
    {
        std::stringstream sqlCommand;
        const char * type = !reverse ? "ASC" : "DESC";
        sqlCommand << "select * from " << tab << " ORDER BY " << field << " " << type << " LIMIT " << count;
        return this->Invoke(tab, sqlCommand.str(), taskSource);
    }
}