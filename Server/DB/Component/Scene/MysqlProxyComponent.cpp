#include "MysqlProxyComponent.h"

#include <Core/App.h>
#include <Service/RpcNodeProxy.h>

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
        db::UserAccountData userAccountData;
        userAccountData.set_userid(12345678);
        userAccountData.set_account("123456@qq.com");
        userAccountData.set_devicemac("ios_qq");
        userAccountData.set_token("sadhaihdi2h3ihdisaxnkfjs");
        userAccountData.set_registertime(TimeHelper::GetSecTimeStamp());

        XCode code = this->Add(userAccountData);

        db::UserAccountData userAccountData2;
        userAccountData2.set_userid(12345678);
        userAccountData2.set_account("123456@qq.com");

        XCode code2 = this->Query(userAccountData2, userAccountData2);

        GKDebugLogProtocBuf(userAccountData2);
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