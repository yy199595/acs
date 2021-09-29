#include "MysqlProxyComponent.h"

#include <Core/App.h>
#include <Service/ServiceNode.h>
#include <Coroutine/CoroutineComponent.h>


namespace Sentry
{
    bool MysqlProxyComponent::Awake()
    {
        this->mMysqlProxyNodeId = -1;
		this->mCorComponent = App::Get().GetCoroutineComponent();     
        SayNoAssertRetFalse_F(this->mNodeManager = Scene::GetComponent<ServiceNodeComponent>());

        return true;
    }

    void MysqlProxyComponent::Start()
    {

    }

    void MysqlProxyComponent::OnFrameUpdate(float t)
    {
        while (!this->mWakeUpQueue.empty() && this->GetServiceNode())
        {
            const long long id = this->mWakeUpQueue.front();
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

    XCode MysqlProxyComponent::Add(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorComponent->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }

        std::string messageData;
        s2s::MysqlOper_Request *requestData = this->mMysqlOperReqPool.Create();
        s2s::MysqlOper_Response *responseData = this->mMysqlOperResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlService", "Add", *requestData, *responseData);
#ifdef SOEASY_DEBUG
        if (code != XCode::Successful)
        {
            SayNoDebugError("[mysql error] : " << responseData->errorstr());
        }
#endif
        this->mMysqlOperReqPool.Destory(requestData);
        this->mMysqlOperResPool.Destory(responseData);
        return code;
    }

    XCode MysqlProxyComponent::Query(const Message &data, Message &queryData)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorComponent->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }

        std::string messageData;
        s2s::MysqlQuery_Request *requestData = this->mMysqlQueryReqPool.Create();
        s2s::MysqlQuery_Response *responseData = this->mMysqlQueryResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlService", "Query", *requestData, *responseData);
#ifdef SOEASY_DEBUG
        if (code != XCode::Successful)
        {
            SayNoDebugError("[mysql error] : " << responseData->errotstr());
        }
#endif
        if (code == XCode::Successful)
        {
            if (responseData->querydatas_size() > 0)
            {
                const std::string &data = responseData->querydatas(0);
                queryData.ParseFromString(data);
            }
        }
        this->mMysqlQueryReqPool.Destory(requestData);
        this->mMysqlQueryResPool.Destory(responseData);
        return XCode::Failure;
    }

    XCode MysqlProxyComponent::Save(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorComponent->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }
        std::string messageData;
        s2s::MysqlOper_Request *requestData = this->mMysqlOperReqPool.Create();
        s2s::MysqlOper_Response *responseData = this->mMysqlOperResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlService", "Save", *requestData, *responseData);
#ifdef SOEASY_DEBUG
        if (code != XCode::Successful)
        {
            SayNoDebugError("[mysql error] : " << responseData->errorstr());
        }
#endif
        this->mMysqlOperReqPool.Destory(requestData);
        this->mMysqlOperResPool.Destory(responseData);
        return XCode::Failure;
    }

    XCode MysqlProxyComponent::Delete(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorComponent->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorComponent->YieldReturn();
        }
        std::string messageData;
        s2s::MysqlOper_Request *requestData = this->mMysqlOperReqPool.Create();
        s2s::MysqlOper_Response *responseData = this->mMysqlOperResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlService", "Delete", *requestData, *responseData);
#ifdef SOEASY_DEBUG
        if (code != XCode::Successful)
        {
            SayNoDebugError("[mysql error] : " << responseData->errorstr());
        }
#endif
        this->mMysqlOperReqPool.Destory(requestData);
        this->mMysqlOperResPool.Destory(responseData);
        return code;
    }
}