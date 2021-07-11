#include "MysqlProxyManager.h"
#include <Other/ServiceNode.h>
#include <Coroutine/CoroutineManager.h>
#include <Manager/ServiceNodeManager.h>
#include <Protocol/s2s.pb.h>
namespace SoEasy
{
    bool MysqlProxyManager::OnInit()
    {
        this->mMysqlProxyNodeId = -1;
        SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
        SayNoAssertRetFalse_F(this->mNodeManager = this->GetManager<ServiceNodeManager>());

        return true;
    }

    void MysqlProxyManager::OnInitComplete()
    {

    }

    void MysqlProxyManager::OnFrameUpdate(float t)
    {
        while (!this->mWakeUpQueue.empty() && this->GetServiceNode())
        {
            const long long id = this->mWakeUpQueue.front();
            this->mCorManager->Resume(id);
            this->mWakeUpQueue.pop();
        }
    }

    ServiceNode *MysqlProxyManager::GetServiceNode()
    {
        ServiceNode *proxyNode = this->mNodeManager->GetServiceNode(this->mMysqlProxyNodeId);
        if (proxyNode == nullptr)
        {
            proxyNode = this->mNodeManager->GetNodeByServiceName("MysqlProxy");
            this->mMysqlProxyNodeId = proxyNode != nullptr ? proxyNode->GetNodeId() : -1;
            return proxyNode;
        }
        return proxyNode;
    }

    XCode MysqlProxyManager::Add(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorManager->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorManager->YieldReturn();
        }

        std::string messageData;
        s2s::MysqlOper_Request *requestData = this->mMysqlOperReqPool.Create();
        s2s::MysqlOper_Response *responseData = this->mMysqlOperResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlProxy", "Insert", *requestData, *responseData);
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

    XCode MysqlProxyManager::Query(const Message &data, Message &queryData)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorManager->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorManager->YieldReturn();
        }

        std::string messageData;
        s2s::MysqlQuery_Request *requestData = this->mMysqlQueryReqPool.Create();
        s2s::MysqlQuery_Response *responseData = this->mMysqlQueryResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlProxy", "Save", *requestData, *responseData);
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

    XCode MysqlProxyManager::Save(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorManager->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorManager->YieldReturn();
        }
        std::string messageData;
        s2s::MysqlOper_Request *requestData = this->mMysqlOperReqPool.Create();
        s2s::MysqlOper_Response *responseData = this->mMysqlOperResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlProxy", "Save", *requestData, *responseData);
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

    XCode MysqlProxyManager::Delete(const Message &data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode == nullptr)
        {
            long long id = this->mCorManager->GetCurrentCorId();
            this->mWakeUpQueue.push(id);
            this->mCorManager->YieldReturn();
        }
        std::string messageData;
        s2s::MysqlOper_Request *requestData = this->mMysqlOperReqPool.Create();
        s2s::MysqlOper_Response *responseData = this->mMysqlOperResPool.Create();
        if (requestData->SerializeToString(&messageData))
        {
            requestData->set_protocolname(data.GetTypeName());
            requestData->set_protocolmessage(messageData);
        }
        XCode code = proxyNode->Call("MysqlProxy", "Delete", *requestData, *responseData);
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