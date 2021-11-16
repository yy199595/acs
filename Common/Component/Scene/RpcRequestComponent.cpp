#include "RpcRequestComponent.h"
#include <Service/LocalServiceComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Util/StringHelper.h>
#include <Core/App.h>
#include <Scene/RpcProtoComponent.h>
#include <Pool/MessagePool.h>
#include <Method/LuaServiceMethod.h>
#include <Component/Scene/RpcComponent.h>
namespace GameKeeper
{
    bool RpcRequestComponent::Awake()
    {
        this->mCorComponent = App::Get().GetCorComponent();
        const ServerConfig & ServerCfg = App::Get().GetConfig();
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
#ifdef __DEBUG__
        GKDebugWarning("call " << protocolConfig->Service << "." << protocolConfig->Method);
#endif
        const std::string &service = protocolConfig->Service;
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
            method->SetSocketId(request.socketid());
			auto response = new com::Rpc_Response();
            XCode code = method->Invoke(request, *response);
            if (request.rpcid() != 0)
            {       			
				response->set_code(code);
				response->set_rpcid(request.rpcid());
				response->set_userid(request.userid());
                this->mRpcComponent->SendByAddress(request.socketid(), response);
            }
			delete &request;
        }
        else if(method->IsLuaMethod()) //lua 异步
        {

        }
        else
        {			
            this->mCorComponent->StartCoroutine(&RpcRequestComponent::Invoke, this, method, &request);
        }
        return true;
    }

	void RpcRequestComponent::Invoke(ServiceMethod * method, const com::Rpc_Request * request)
    {        
		auto response = new com::Rpc_Response();
        XCode code = method->Invoke(*request, *response);
		if (request->rpcid() != 0)
		{
			response->set_code(code);
			response->set_rpcid(request->rpcid());
			response->set_userid(request->userid());
			this->mRpcComponent->SendByAddress(request->socketid(), response);
		}
		delete request;
    }
}// namespace GameKeeper
