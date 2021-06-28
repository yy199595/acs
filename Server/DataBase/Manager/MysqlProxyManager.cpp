#include "MysqlProxyManager.h"
#include <Other/ServiceNode.h>
#include <Coroutine/CoroutineManager.h>
#include <Manager/ServiceNodeManager.h>
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

    void MysqlProxyManager::OnSystemUpdate()
    {
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

    XCode MysqlProxyManager::Add(shared_ptr<Message> data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode != nullptr)
        {
            return proxyNode->Invoke("MysqlProxy", "Insert", data);
        }
        this->mCorManager->YieldReturn();
        return XCode::Failure;
    }

    XCode MysqlProxyManager::Query(shared_ptr<Message> data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode != nullptr)
        {
            return proxyNode->Call("MysqlProxy", "Query", data, data);
        }
        this->mCorManager->YieldReturn();
        return XCode::Failure;
    }

    XCode MysqlProxyManager::Update(shared_ptr<Message> data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode != nullptr)
        {
            return proxyNode->Invoke("MysqlProxy", "Insert", data);
        }
        this->mCorManager->YieldReturn();
        return XCode::Failure;
    }

    XCode MysqlProxyManager::Delete(shared_ptr<Message> data)
    {
        ServiceNode *proxyNode = this->GetServiceNode();
        if (proxyNode != nullptr)
        {
            return proxyNode->Invoke("MysqlProxy", "Insert", data);
        }
        this->mCorManager->YieldReturn();
        return XCode::Failure;
    }
}