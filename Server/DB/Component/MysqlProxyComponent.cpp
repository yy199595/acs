#include "MysqlProxyComponent.h"

#include"Core/App.h"
#include"Service/RpcNode.h"
#include"Pool/MessagePool.h"
#include"Util/StringHelper.h"
#include"Util/MathHelper.h"
#include"Service/NodeProxyComponent.h"
#include"Other/ElapsedTimer.h"
#include"MysqlClient/MysqlRpcTask.h"
namespace GameKeeper
{
    bool MysqlProxyComponent::Awake()
    {
        this->mMysqlNodeId = -1;
		this->mCorComponent = nullptr;
        return true;
    }

    bool MysqlProxyComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());
        return true;
    }

    void MysqlProxyComponent::OnFrameUpdate(float t)
    {
        if(this->mMysqlNodeId == -1)
        {
            return;
        }
    }

    void MysqlProxyComponent::OnLoadData()
    {
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
            this->mMysqlNodeId = node->GetGlobalId();
        }
    }

    void MysqlProxyComponent::OnDelRpcNode(RpcNode *node)
    {
        if(this->mMysqlNodeId == node->GetGlobalId())
        {
            this->mMysqlNodeId = -1;
        }
    }

	void MysqlProxyComponent::AddUserData()
	{
		long long userId = MathHelper::Random(10000, 10000000);
		for (int index = 0; index < 10; index++)
        {
            int random = MathHelper::Random<int>(100, 100000);
            db::UserAccountData userAccountData;
            userAccountData.set_userid(userId + random);
            userAccountData.set_account(std::to_string(userId + index) + "@qq.com");
            userAccountData.set_devicemac("ios_qq");
            userAccountData.set_token(StringHelper::CreateNewToken());
            userAccountData.set_registertime(Helper::Time::GetSecTimeStamp());
            if (this->Add(userAccountData)->AwakeGetCode() == XCode::Successful)
            {
                LOG_WARN("add user data successful");
            }
            //this->mCorComponent->AwaitSleep(100);
        }
	}

	void MysqlProxyComponent::SortUserData()
	{
		for (int index = 0; index < 100; index++)
		{
			int count = MathHelper::Random(3, 20);
			auto sortTask = this->Sort("tb_player_account", "UserID", count);
			if (sortTask->AwakeGetCode() == XCode::Successful)
            {
                for (size_t index = 0; index < sortTask->AwaitGetDataSize(); index++)
                {
                    std::string json;
                    auto userData = sortTask->AwaitGetData<db::UserAccountData>();
                    util::MessageToJsonString(*userData, &json);
                    LOG_WARN(index++ << "  " << json);
                }
            }
			//this->mCorComponent->AwaitSleep(100);
		}
	}

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Add(const Message &data)
    {
        RpcNode *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = proxyNode->NewRequest("MysqlService.Add", methodId);
        if(requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Add");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }

        std::shared_ptr<MysqlRpcTask> rpcTask(new MysqlRpcTask(methodId));

        this->mOperRequest.Clear();
        requestMessage->set_rpcid(rpcTask->GetTaskId());
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        return rpcTask;
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Query(const Message &data)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Query", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Query");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        std::shared_ptr<MysqlRpcTask> rpcTask(new MysqlRpcTask(methodId));

        this->mQueryRequest.Clear();
        requestMessage->set_rpcid(rpcTask->GetTaskId());
        this->mQueryRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mQueryRequest);
        return rpcTask;
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Invoke", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Invoke");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        std::shared_ptr<MysqlRpcTask> rpcTask(new MysqlRpcTask(methodId));

        this->mAnyOperRequest.Clear();
        this->mAnyOperRequest.set_sql(sql);
        this->mAnyOperRequest.set_tab(tab);
        requestMessage->set_rpcid(rpcTask->GetTaskId());
        requestMessage->mutable_data()->PackFrom(this->mAnyOperRequest);
        return rpcTask;
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Save(const Message &data)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Save", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Save");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        std::shared_ptr<MysqlRpcTask> rpcTask(new MysqlRpcTask(methodId));

        this->mOperRequest.Clear();
        requestMessage->set_rpcid(rpcTask->GetTaskId());
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        return rpcTask;
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Delete(const Message &data)
    {
        RpcNode *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->NewRequest("MysqlService.Delete", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Delete");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        std::shared_ptr<MysqlRpcTask> rpcTask(new MysqlRpcTask(methodId));

        this->mOperRequest.Clear();
        requestMessage->set_rpcid(rpcTask->GetTaskId());
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        return rpcTask;
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Sort(const std::string &tab, const std::string &field, int count, bool reverse)
    {
        std::stringstream sqlCommand;
        const char * type = !reverse ? "ASC" : "DESC";
        sqlCommand << "select * from " << tab << " ORDER BY " << field << " " << type << " LIMIT " << count;
        return this->Invoke(tab, sqlCommand.str());
    }
}