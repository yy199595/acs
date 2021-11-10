#include "RpcRequestComponent.h"
#include <Service/LocalServiceComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Util/StringHelper.h>
#include <Core/App.h>
#include <Util/JsonHelper.h>
#include <Scene/RpcProtoComponent.h>
#include <Pool/MessagePool.h>
#include <Method/LuaServiceMethod.h>
#include <Network/Rpc/RpcComponent.h>
namespace GameKeeper
{
    bool RpcRequestComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		this->mCorComponent = App::Get().GetCorComponent();
        GKAssertRetFalse_F(ServerCfg.GetValue("NodeId", this->mNodeId));
        GKAssertRetFalse_F(this->mRpcComponent = this->GetComponent<RpcComponent>());
        GKAssertRetFalse_F(this->mCorComponent = this->GetComponent<CoroutineComponent>());
        GKAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<RpcProtoComponent>());
        return true;
    }

    bool RpcRequestComponent::OnRequest(const com::Rpc_Request & request)
    {
        unsigned short methodId = request.methodid();
        const ProtocolConfig *protocolConfig = this->mProtocolComponent->GetProtocolConfig(methodId);
        if (protocolConfig == nullptr)
        {
            return false;
        }
        const std::string &service = protocolConfig->ServiceName;
        auto logicService = this->gameObject->GetComponent<ServiceComponent>(service);
        if (logicService == nullptr)
        {
            GKDebugFatal("call service not exist : [" << service << "]");
            return false;
        }

        const std::string &methodName = protocolConfig->Method;
        ServiceMethod *method = logicService->GetMethod(methodName);
        if (service == nullptr)
        {
            GKDebugFatal("call method not exist : [" << service << "." << methodName << "]");
            return false;
        }

        if (!protocolConfig->IsAsync)
        {
            std::string responseContent;
            //method->SetAddress(request.address()); //TODO
            XCode code = method->Invoke(request, responseContent);
            if (request.rpcid() != 0)
            {
                this->mResponse.Clear();
                this->mResponse.set_code(code);
                this->mResponse.set_rpcid(request.rpcid());
                this->mResponse.set_userid(request.userid());
                this->mResponse.set_methodid(request.methodid());
                this->mResponse.set_messagedata(responseContent);
                this->mRpcComponent->SendByAddress(request.socketid(), this->mResponse);
            }
        }
        else if(method->IsLuaMethod()) //lua 异步
        {

        }
        else
        {
			com::Rpc_Request * requestData = this->mRequestDataPool.CopyFrom(request);
            this->mCorComponent->StartCoroutine(&RpcRequestComponent::Invoke, this, method, requestData);
        }
        return true;
    }

	void RpcRequestComponent::Invoke(ServiceMethod * method, com::Rpc_Request * request)
    {
        std::string responseContent;
        //method->SetAddress(request->address());
        XCode code = method->Invoke(*request, responseContent);
        if (request->rpcid() != 0)
        {
            this->mResponse.Clear();
            this->mResponse.set_code(code);
            this->mResponse.set_rpcid(request->rpcid());
            this->mResponse.set_userid(request->userid());
            this->mResponse.set_methodid(request->methodid());
            this->mResponse.set_messagedata(responseContent);
            this->mRpcComponent->SendByAddress(request->socketid(), this->mResponse);

			this->mRequestDataPool.Destory(request);
        }
    }
}// namespace GameKeeper
