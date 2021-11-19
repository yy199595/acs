#include "MysqlProxyComponent.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>
#include"Pool/MessagePool.h"
#include"Util/StringHelper.h"
#include "Util/MathHelper.h"
namespace GameKeeper
{
    bool MysqlProxyComponent::Awake()
    {
        this->mMysqlNodeId = -1;
		this->mCorComponent = App::Get().GetCorComponent();
        GKAssertRetFalse_F(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());

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

    void MysqlProxyComponent::OnLodaData()
    {
//        XCode code0 = this->Invoke("tb_player_account", "delete from tb_player_account");
//
//        long long userId = 10000;
//        for (int index = 0; index < 100; index++)
//        {
//            int random = MathHelper::Random<int>(100, 100000);
//            db::UserAccountData userAccountData;
//            userAccountData.set_userid(userId + random);
//            userAccountData.set_account(std::to_string(userId + index) + "@qq.com");
//            userAccountData.set_devicemac("ios_qq");
//            userAccountData.set_token(StringHelper::CreateNewToken());
//            userAccountData.set_registertime(TimeHelper::GetSecTimeStamp());
//            XCode code = this->Add(userAccountData);
//        }

        std::vector<Message *> responses;
        XCode code3 = this->Invoke("tb_player_account", "select * from tb_player_account ORDER BY UserID", responses);
        if (code3 == XCode::Successful)
        {
            for (const Message *message: responses)
            {
                std::string json;
                util::MessageToJsonString(*message, &json);
                GKDebugWarning(json);
            }
        }
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

    XCode MysqlProxyComponent::Add(const Message &data)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        s2s::MysqlOper_Request requestData;
        requestData.mutable_data()->PackFrom(data);
        return proxyNode->Invoke("MysqlService.Add", requestData);
    }

    XCode MysqlProxyComponent::Query(const Message &data, Message &queryData)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        s2s::MysqlQuery_Request requestData;
        s2s::MysqlQuery_Response responseData;
        requestData.mutable_data()->PackFrom(data);
        XCode code = proxyNode->Call("MysqlService.Query", requestData, responseData);
        if (code == XCode::Successful && responseData.querydatas_size() > 0)
        {
            const Any &data = responseData.querydatas(0);
            if (!data.UnpackTo(&queryData))
            {
                return XCode::ParseMessageError;
            }
        }
        return code;
    }

    XCode MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        s2s::MysqlAnyOper_Request requestData;
        s2s::MysqlAnyOper_Response responseData;

        requestData.set_sql(sql);
        requestData.set_tab(tab);
        return proxyNode->Call("MysqlService.Invoke", requestData, responseData);
    }

    XCode MysqlProxyComponent::Invoke(const std::string &tab, const std::string &sql,
                                      std::vector<Message *> &queryDatas)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        s2s::MysqlAnyOper_Request requestData;
        s2s::MysqlAnyOper_Response responseData;

        requestData.set_sql(sql);
        requestData.set_tab(tab);
        XCode code = proxyNode->Call("MysqlService.Invoke", requestData, responseData);
        if (code == XCode::Successful && responseData.querydatas_size() > 0)
        {
            for (int index = 0; index < responseData.querydatas_size(); index++)
            {
                const Any &any = responseData.querydatas(index);
                Message *message = MessagePool::New(any);
                if(message == nullptr)
                {
                    return XCode::ParseMessageError;
                }
                Message * newMessage = message->New();
                if(!any.UnpackTo(newMessage))
                {
                    delete newMessage;
                    return XCode::ParseMessageError;
                }
                queryDatas.emplace_back(newMessage);
            }
        }
        return code;
    }

    XCode MysqlProxyComponent::Save(const Message &data)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        s2s::MysqlOper_Request requestData;
        requestData.mutable_data()->PackFrom(data);
        return proxyNode->Invoke("MysqlService.Save", requestData);
    }

    XCode MysqlProxyComponent::Delete(const Message &data)
    {
        RpcNodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlNodeId);
        if (proxyNode == nullptr)
        {
            return XCode::CallServiceNotFound;
        }
        s2s::MysqlOper_Request requestData;
        requestData.mutable_data()->PackFrom(data);
        return proxyNode->Invoke("MysqlService.Delete", requestData);
    }
}