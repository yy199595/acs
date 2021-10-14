#include "ServiceMgrComponent.h"
#include <Service/LocalServiceComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Util/StringHelper.h>
#include <Core/App.h>
#include <Util/JsonHelper.h>
#include <Scene/ProtocolComponent.h>
#include <Pool/MessagePool.h>
#include <NetWork/LuaServiceMethod.h>
namespace Sentry
{
    bool ServiceMgrComponent::Awake()
    {
		ServerConfig & ServerCfg = App::Get().GetConfig();
		this->mCorComponent = App::Get().GetCoroutineComponent();
        SayNoAssertRetFalse_F(ServerCfg.GetValue("NodeId", this->mNodeId));

        SayNoAssertRetFalse_F(this->mCorComponent = this->GetComponent<CoroutineComponent>());
        SayNoAssertRetFalse_F(this->mNetProxyManager = this->GetComponent<NetProxyComponent>());
        SayNoAssertRetFalse_F(this->mProtocolComponent = this->GetComponent<ProtocolComponent>());
        return true;
    }

    bool ServiceMgrComponent::OnRequestMessage(const std::string & address, SharedMessage message)
    {
        unsigned short methodId = 0;
        const char *ptr = message->c_str();
        memcpy(&methodId, ptr + 1, sizeof(methodId));

        const ProtocolConfig *protocolConfig = this->mProtocolComponent->GetProtocolConfig(methodId);
        if (protocolConfig == nullptr)
        {
            return false;
        }
        const std::string &service = protocolConfig->ServiceName;
        auto logicService = this->gameObject->GetComponent<ServiceComponent>(service);
        if (logicService == nullptr)
        {
            SayNoDebugFatal("call service not exist : [" << service << "]");
            return false;
        }

        const std::string &methodName = protocolConfig->Method;
        ServiceMethod *method = logicService->GetMethod(methodName);
        if (service == nullptr)
        {
            SayNoDebugFatal("call method not exist : [" << service << "." << methodName << "]");
            return false;
        }

        if (!protocolConfig->IsAsync)
        {
            com::DataPacket_Request requestMessage;
            if(!requestMessage.ParseFromArray(ptr + 3, message->size() -3))
            {
                return false;
            }
            std::string responseContent;
            method->SetAddress(address);
            XCode code = method->Invoke(requestMessage, responseContent);
            if (requestMessage.rpcid() != 0)
            {
                com::DataPacket_Response responseMessage;

                responseMessage.set_code(code);
                responseMessage.set_messagedata(responseContent);
                responseMessage.set_userid(requestMessage.userid());
                responseMessage.set_methodid(requestMessage.methodid());
                this->mNetProxyManager->SendNetMessage(address, responseMessage);
            }
        }
        else if(method->IsLuaMethod()) //lua 异步
        {

        }
        else
        {
            std::string add = address;
            this->mCorComponent
                    ->StartCoroutine(&ServiceMgrComponent::Invoke, this, method, add, message);
        }
        return true;
    }

	void ServiceMgrComponent::Invoke(ServiceMethod * method, std::string &address, SharedMessage message)
    {
        const char * ptr = message->c_str();
        com::DataPacket_Request requestMessage;
        if(!requestMessage.ParseFromArray(ptr + 3, message->size() -3))
        {
            return;
        }
        std::string responseContent;
        method->SetAddress(address);
        XCode code = method->Invoke(requestMessage, responseContent);
        if (requestMessage.rpcid() != 0)
        {
            com::DataPacket_Response responseMessage;

            responseMessage.set_code(code);
            responseMessage.set_messagedata(responseContent);
            responseMessage.set_userid(requestMessage.userid());
            responseMessage.set_methodid(requestMessage.methodid());
            this->mNetProxyManager->SendNetMessage(address, responseMessage);
        }
    }
}// namespace Sentry
