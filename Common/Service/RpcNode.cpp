#include"RpcNode.h"
#include<Core/App.h>
#include<Coroutine/TaskComponent.h>
#include"Async/RpcTask/RpcTaskSource.h"
#include<Rpc/RpcClientComponent.h>
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include"Rpc/RpcComponent.h"
namespace GameKeeper
{
	RpcNode::RpcNode(int id)
		: mGlobalId(id), mIsClose(false), mSocketId(0)
	{
        this->mRpcComponent = App::Get().GetComponent<RpcComponent>();
        this->mRpcClientComponent = App::Get().GetComponent<RpcClientComponent>();
        LOG_CHECK_RET(this->mTaskComponent = App::Get().GetComponent<TaskComponent>());
        LOG_CHECK_RET(this->mRpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>());
        this->mTaskComponent->Start(&RpcNode::LoopSend, this);
	}

	bool RpcNode::AddService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		if (iter == this->mServiceArray.end())
		{
			this->mServiceArray.insert(service);
			return true;
		}
		return false;
	}

    void RpcNode::LoopSend()
    {
        this->mLoopTaskSource = std::make_shared<LoopTaskSource>();
        while (!this->mIsClose)
        {
            if (!this->mSendQueue.empty())
            {
                auto clientSession = this->mRpcClientComponent->GetRpcSession(this->mSocketId);
                if (!clientSession->IsOpen())
                {
                    int connectCount = 0;
                    const std::string &ip = this->mNodeIp;
                    const unsigned short port = this->mNodePort;
                    while (!clientSession->ConnectAsync(ip, port)->Await())
                    {
                        connectCount++;
                        LOG_ERROR("connect ", this->mNodeName, " => ", ip, ':', port, " failure count = ",
                                  connectCount);
                        this->mTaskComponent->Sleep(3000);
                    }
                }
                while (!this->mSendQueue.empty())
                {
                    auto requestMessage = this->mSendQueue.front();
                    this->mSendQueue.pop();
                    clientSession->SendToServer(requestMessage);
                }
            }
            this->mLoopTaskSource->Await();
        }
    }

    bool RpcNode::UpdateNodeProxy(const s2s::NodeInfo &nodeInfo)
    {
        auto iter = nodeInfo.listeners().find("rpc");
        if(iter == nodeInfo.listeners().end())
        {
            return false;
        }

        this->mServiceArray.clear();
        this->mAreaId = nodeInfo.area_id();
        this->mNodeId = nodeInfo.node_id();
        this->mNodeIp = nodeInfo.server_ip();
        this->mNodeName = nodeInfo.server_name();
        this->mNodePort = (unsigned short)iter->second;
        for(const std::string & name : nodeInfo.services())
        {
            this->mServiceArray.insert(name);
        }
        this->mNodeInfo.CopyFrom(nodeInfo);
        return true;
    }


    void RpcNode::Destory()
    {
        delete this;
    }

    bool RpcNode::HasService(const std::string &service)
	{
		auto iter = this->mServiceArray.find(service);
		return iter != this->mServiceArray.end();
	}

    void RpcNode::GetServices(std::vector<std::string> & services)
    {
        services.clear();
        for(const std::string & name : this->mServiceArray)
        {
            services.emplace_back(name);
        }
    }

	std::shared_ptr<com::Rpc_Request> RpcNode::NewRequest(const std::string & method)
	{
		auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
		if (config == nullptr)
		{
			return nullptr;
		}
        std::shared_ptr<com::Rpc_Request> request(new com::Rpc_Request());
		request->set_method_id(config->MethodId);
        if(this->mSocketId == 0)
        {
           this->mSocketId = this->mRpcClientComponent->MakeSession(this->mNodeName);
        }
        this->mSendQueue.push(request);
        if(this->mLoopTaskSource != nullptr)
        {
            this->mLoopTaskSource->SetResult();
        }
        return request;
	}

    XCode RpcNode::Call(const string &func, std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("{0} not config ", func);
            return XCode::NotFoundRpcConfig;
        }
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }

    XCode RpcNode::Call(const string &func, const Message &message, std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("{0} not rpc config ", func);
            return XCode::NotFoundRpcConfig;
        }
        requestData->mutable_data()->PackFrom(message);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
            return taskSource->GetCode();
        }
        return XCode::Successful;
    }
}// namespace GameKeeper