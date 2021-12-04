#include "MysqlProxyComponent.h"

#include<Core/App.h>
#include<Service/RpcNodeProxy.h>
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
		this->mCorComponent = App::Get().GetCorComponent();
        LOG_CHECK_RET_FALSE(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());

        return true;
    }

    void MysqlProxyComponent::Start()
    {

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
//			group->Add(this->mCorComponent->StartCoroutine(&MysqlProxyComponent::AddUserData, this));
//			group->Add(this->mCorComponent->StartCoroutine(&MysqlProxyComponent::SortUserData, this));
//		}
//		group->AwaitAll();
//		LOG_ERROR("use time = " << timer.GetSecond() << "s");
    }

    void MysqlProxyComponent::OnAddProxyNode(RpcNodeProxy *node)
    {
        if(node->HasService("MysqlService"))
        {
            this->mMysqlNodeId = node->GetGlobalId();
        }
    }

    void MysqlProxyComponent::OnDelProxyNode(RpcNodeProxy *node)
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
            userAccountData.set_registertime(TimeHelper::GetSecTimeStamp());
            if (this->Add(userAccountData)->AwakeGetCode() == XCode::Successful)
            {
                LOG_WARN("add user data successful");
            }
            //this->mCorComponent->WaitForSleep(100);
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
			//this->mCorComponent->WaitForSleep(100);
		}
	}

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Add(const Message &data)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = proxyNode->CreateProtoRequest("MysqlService.Add", methodId);
        if(requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Add");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        proxyNode->SendRequestData(requestMessage);
        return std::make_shared<MysqlRpcTask>(methodId, requestMessage->rpcid());
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Query(const Message &data)
    {
        RpcNodeProxy *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->CreateProtoRequest("MysqlService.Query", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Query");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->mQueryRequest.Clear();
        this->mQueryRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mQueryRequest);
        mysqlServiceNode->SendRequestData(requestMessage);
        return std::make_shared<MysqlRpcTask>(methodId, requestMessage->rpcid());
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql)
    {
        RpcNodeProxy *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->CreateProtoRequest("MysqlService.Invoke", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Invoke");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->mAnyOperRequest.Clear();
        this->mAnyOperRequest.set_sql(sql);
        this->mAnyOperRequest.set_tab(tab);
        requestMessage->mutable_data()->PackFrom(this->mAnyOperRequest);
        mysqlServiceNode->SendRequestData(requestMessage);
        return std::make_shared<MysqlRpcTask>(methodId, requestMessage->rpcid());
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Save(const Message &data)
    {
        RpcNodeProxy *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->CreateProtoRequest("MysqlService.Save", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Save");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        mysqlServiceNode->SendRequestData(requestMessage);
        return std::make_shared<MysqlRpcTask>(methodId, requestMessage->rpcid());
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Delete(const Message &data)
    {
        RpcNodeProxy *mysqlServiceNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (mysqlServiceNode == nullptr)
        {
            LOG_ERROR("not find mysql service node " << this->mMysqlNodeId);
            return std::make_shared<MysqlRpcTask>(XCode::CallServiceNotFound);
        }
        int methodId = 0;
        auto requestMessage = mysqlServiceNode->CreateProtoRequest("MysqlService.Delete", methodId);
        if (requestMessage == nullptr)
        {
            LOG_ERROR("not find mysql service method MysqlService.Delete");
            return std::make_shared<MysqlRpcTask>(XCode::NotFoundRpcConfig);
        }
        this->mOperRequest.Clear();
        this->mOperRequest.mutable_data()->PackFrom(data);
        requestMessage->mutable_data()->PackFrom(this->mOperRequest);
        return std::make_shared<MysqlRpcTask>(methodId, requestMessage->rpcid());
    }

    std::shared_ptr<MysqlRpcTask> MysqlProxyComponent::Sort(const std::string &tab, const std::string &field, int count, bool reverse)
    {
        std::stringstream sqlCommand;
        const char * type = !reverse ? "ASC" : "DESC";
        sqlCommand << "select * from " << tab << " ORDER BY " << field << " " << type << " LIMIT " << count;
        return this->Invoke(tab, sqlCommand.str());
    }
}