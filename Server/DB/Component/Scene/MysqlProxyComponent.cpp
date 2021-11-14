#include "MysqlProxyComponent.h"

#include <Core/App.h>
#include <Service/NodeProxy.h>

namespace GameKeeper
{
    bool MysqlProxyComponent::Awake()
    {
        this->mMysqlProxyNodeId = -1;
		this->mCorComponent = App::Get().GetCorComponent();
        GKAssertRetFalse_F(this->mNodeComponent = this->GetComponent<NodeProxyComponent>());

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

    NodeProxy *MysqlProxyComponent::GetServiceNode()
    {
        NodeProxy *proxyNode = this->mNodeComponent->GetServiceNode(this->mMysqlProxyNodeId);
        if (proxyNode == nullptr)
        {
            proxyNode = this->mNodeComponent->GetNodeByServiceName("MysqlService");
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
        NodeProxy *proxyNode = this->GetServiceNode();
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
		requestData.mutable_data()->PackFrom(data);
        return proxyNode->Invoke("MysqlService.Add", requestData);
    }

    XCode MysqlProxyComponent::Query(const Message &data, Message &queryData)
    {
        NodeProxy *proxyNode = this->GetServiceNode();
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
      
        s2s::MysqlQuery_Request requestData;
        s2s::MysqlQuery_Response responseData;
		requestData.mutable_data()->PackFrom(data);
        XCode code = proxyNode->Call("MysqlService.Add", requestData, responseData);
        if (code == XCode::Successful && responseData.querydatas_size() > 0)
        {
            const Any & data = responseData.querydatas(0);
			if (!data.UnpackTo(&queryData))
			{
				return XCode::ParseMessageError;
			}          
        }
        return code;
    }

    XCode MysqlProxyComponent::Save(const Message &data)
    {
        NodeProxy *proxyNode = this->GetServiceNode();
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
		requestData.mutable_data()->PackFrom(data);
        return proxyNode->Invoke("MysqlService.Add", requestData);
    }

    XCode MysqlProxyComponent::Delete(const Message &data)
    {
        NodeProxy *proxyNode = this->GetServiceNode();
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
		requestData.mutable_data()->PackFrom(data);   
        return proxyNode->Invoke("MysqlService.Delete",requestData);
    }
}