#include "MysqlProxyComponent.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>

namespace GameKeeper
{
    bool MysqlProxyComponent::Awake()
    {
        this->mMysqlProxyNodeId = -1;
		this->mCorComponent = App::Get().GetCorComponent();
        GKAssertRetFalse_F(this->mNodeManager = this->GetComponent<ServiceNodeComponent>());

        return true;
    }

    void MysqlProxyComponent::Start()
    {

    }

    void MysqlProxyComponent::OnFrameUpdate(float t)
    {
        while (!this->mWakeUpQueue.empty() && this->GetServiceNode())
        {
            unsigned int id = this->mWakeUpQueue.front();
            this->mCorComponent->Resume(id);
            this->mWakeUpQueue.pop();
        }
    }

    ServiceNode *MysqlProxyComponent::GetServiceNode()
    {
        ServiceNode *proxyNode = this->mNodeManager->GetServiceNode(this->mMysqlProxyNodeId);
        if (proxyNode == nullptr)
        {
            proxyNode = this->mNodeManager->GetNodeByServiceName("MysqlService");
            this->mMysqlProxyNodeId = proxyNode != nullptr ? proxyNode->GetNodeUId() : -1;
            return proxyNode;
        }
        return proxyNode;
    }

    void MysqlProxyComponent::OnLodaData()
    {
//        db::UserAccountData userAccountData;
//        userAccountData.set_userid(12345678);
//        userAccountData.set_account("123456@qq.com");
//        userAccountData.set_devicemac("ios_qq");
//        userAccountData.set_token("sadhaihdi2h3ihdisaxnkfjs");
//        userAccountData.set_registertime(TimeHelper::GetSecTimeStamp());
//
//        XCode code = this->Add(userAccountData);
//
//        db::UserAccountData userAccountData2;
//        userAccountData2.set_userid(12345678);
//        userAccountData2.set_account("123456@qq.com");
//
//        XCode code2 = this->Query(userAccountData2, userAccountData2);
//
//        GKDebugLogProtocBuf(userAccountData2);
    }

    XCode MysqlProxyComponent::Add(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            unsigned int id = this->mCorComponent->GetCurrentCorId();
            if(id == 0)
            {
                return XCode::NoCoroutineContext;
            }
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }

        if (!data.SerializeToString(&mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }
        s2s::MysqlOper_Request requestData;
        requestData.set_protocolname(data.GetTypeName());
        requestData.set_protocolmessage(this->mMessageBuffer);
        return proxyNode->Invoke("MysqlService", "Add", requestData);
    }

    XCode MysqlProxyComponent::Query(const Message &data, Message &queryData)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            unsigned int id = this->mCorComponent->GetCurrentCorId();
            if(id == 0)
            {
                return XCode::NoCoroutineContext;
            }
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }

        if (!data.SerializeToString(&this->mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }

        s2s::MysqlQuery_Request requestData;
        s2s::MysqlQuery_Response responseData;

        requestData.set_protocolname(data.GetTypeName());
        requestData.set_protocolmessage(this->mMessageBuffer);

        XCode code = proxyNode->Call("MysqlService", "Query", requestData, responseData);
        if (code == XCode::Successful && responseData.querydatas_size() > 0)
        {
            const std::string &data = responseData.querydatas(0);
            if(!queryData.ParseFromString(data))
            {
                return XCode::ParseMessageError;
            }
        }
        return code;
    }

    XCode MysqlProxyComponent::Save(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            unsigned int id = this->mCorComponent->GetCurrentCorId();
            if(id == 0)
            {
                return XCode::NoCoroutineContext;
            }
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }
        if(!data.SerializeToString(&this->mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }
        s2s::MysqlOper_Request requestData;
        requestData.set_protocolname(data.GetTypeName());
        requestData.set_protocolmessage(this->mMessageBuffer);
        return proxyNode->Invoke("MysqlService", "Save", requestData);
    }

    XCode MysqlProxyComponent::Delete(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            unsigned int id = this->mCorComponent->GetCurrentCorId();
            if(id == 0)
            {
                return XCode::NoCoroutineContext;
            }
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }
        s2s::MysqlOper_Request requestData;
        if (!data.SerializeToString(&this->mMessageBuffer))
        {
            return XCode::SerializationFailure;
        }

        requestData.set_protocolname(data.GetTypeName());
        requestData.set_protocolmessage(this->mMessageBuffer);
        return proxyNode->Invoke("MysqlService", "Delete", requestData);
    }
}