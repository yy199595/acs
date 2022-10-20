#include"RpcService.h"
#include"App/App.h"
#include"Lua/LuaServiceMethod.h"
#include"Config/ServiceConfig.h"
#include"String/StringHelper.h"
#include"Lua/LuaService.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
#include"Component/ForwardHelperComponent.h"
#include"Component/InnerNetMessageComponent.h"
#ifdef __RPC_DEBUG_LOG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	bool RpcService::LateAwake()
	{
        this->mClientComponent = this->GetComponent<InnerNetComponent>();
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        LOG_CHECK_RET_FALSE(this->mForwardComponent = this->GetComponent<ForwardHelperComponent>());
        LOG_CHECK_RET_FALSE(this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>());
        return true;
	}

	void RpcService::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<RpcService>();
		luaRegister.PushExtensionFunction("Call", Lua::Service::Call);
        luaRegister.PushExtensionFunction("AllotLocation", Lua::Service::AllotLocation);
	}

	bool RpcService::IsStartComplete()
	{
		const std::string & service = this->GetName();
		return this->mLocationComponent->GetHostSize(service) > 0;
	}
}

namespace Sentry
{
	XCode RpcService::Send(const std::string& func, const Message& message)
	{
		std::vector<std::string> locations;
		const std::string & service = this->GetName();
		if(!this->mLocationComponent->GetLocationss(service, locations))
		{
			LOG_ERROR(service <<  " address list empty");
			return XCode::Failure;
		}

		for(const std::string & address : locations)
		{
            if(!this->StartSend(address, func, &message))
            {
                LOG_ERROR("send to " << address << "failure");
            }
		}
		return XCode::Successful;
	}

    bool RpcService::StartSend(const std::string &address, const std::string &func, const Message *message)
    {
        const std::string fullName = fmt::format("{0}.{1}", this->GetName(), func);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            LOG_ERROR("not find [" << fullName << "] config");
            return false;
        }
        if(!methodConfig->Request.empty())
        {
            if(message == nullptr)
            {
                LOG_ERROR("call [" << fullName << "] request is empty");
                return false;
            }
            if(message->GetTypeName() != methodConfig->Request)
            {
                LOG_ERROR("call [" << fullName << "] request type error");
                return false;
            }
        }
        std::shared_ptr<Rpc::Packet> request = std::make_shared<Rpc::Packet>();
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
#ifdef __INNER_MSG_FORWARD__
            request->GetHead().Add("to", address);
#endif
            request->GetHead().Add("func", methodConfig->FullName);
            request->WriteMessage(message);
        }
#ifdef __INNER_MSG_FORWARD__
        std::string target;
        this->mForwardComponent->GetLocation(target);
        return this->mMessageComponent->Send(target, request);
#else
        return this->mMessageComponent->Send(address, request);
#endif
    }

    std::shared_ptr<Rpc::Packet> RpcService::CallAwait(
        const std::string &address, const std::string &func, const Message *message)
    {
        const std::string fullName = fmt::format("{0}.{1}", this->GetName(), func);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            LOG_ERROR("not find [" << fullName << "] config");
            return nullptr;
        }

        if(!methodConfig->Request.empty())
        {
            if(message == nullptr)
            {
                LOG_ERROR("call [" << fullName << "] request is empty");
                return nullptr;
            }
            if(message->GetTypeName() != methodConfig->Request)
            {
                LOG_ERROR("call [" << fullName << "] request type error");
                return nullptr;
            }
        }

        std::shared_ptr<Rpc::Packet> request = std::make_shared<Rpc::Packet>();
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->GetHead().Add("func", func);
#ifdef __INNER_MSG_FORWARD__
            request->GetHead().Add("to", address);
#endif
            if (!methodConfig->Request.empty() && message == nullptr)
            {
                CONSOLE_LOG_ERROR("call " << func << " request error");
                return nullptr;
            }
            request->WriteMessage(message);
        }
#ifdef __INNER_MSG_FORWARD__
        std::string target;
        this->mForwardComponent->GetLocation(target);
        return this->mMessageComponent->Call(target, request);
#else
        return this->mMessageComponent->Call(address, request);
#endif
    }

	XCode RpcService::Send(const std::string& address, const std::string& func, const Message& message)
    {
        if(!this->StartSend(address, func, &message))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	XCode RpcService::Call(const std::string & address, const string& func)
    {
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(address, func);
        if (response == nullptr)
        {
            return XCode::NetWorkError;
        }
        return response->GetCode(XCode::Failure);
    }

	XCode RpcService::Call(const std::string & address, const string& func, const Message& message)
	{
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(address, func, &message);
		if(response == nullptr)
		{
			return XCode::NetWorkError;
		}
        return response->GetCode(XCode::Failure);
    }

	XCode RpcService::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
        std::shared_ptr<Rpc::Packet> data = this->CallAwait(address, func, nullptr);
        if(data == nullptr)
        {
            return XCode::Failure;
        }
        if(data->GetCode(XCode::Failure) == XCode::Successful)
        {
            if (!data->ParseMessage(response.get()))
            {
                return XCode::ParseMessageError;
            }
            return XCode::Successful;
        }
        return data->GetCode(XCode::Failure);
	}


	XCode RpcService::Call(const std::string & address, const string& func, const Message& message,
			std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
        std::shared_ptr<Rpc::Packet> data = this->CallAwait(address, func, &message);
        if(data == nullptr)
        {
            return XCode::Failure;
        }
        if(data->GetCode(XCode::Failure) == XCode::Successful)
        {
            if (!data->ParseMessage(response.get()))
            {
                return XCode::ParseMessageError;
            }
            return XCode::Successful;
        }
        return data->GetCode(XCode::Failure);
	}

}

namespace Sentry
{
    XCode RpcService::Send(long long userId, const std::string &func)
    {
        if(!this->StartSend(userId, func))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    XCode RpcService::Send(long long int userId, const string& func, const Message& message)
    {
        if(!this->StartSend(userId, func, &message))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }


	XCode RpcService::Call(long long userId, const std::string& func)
	{
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(userId, func);
        return response != nullptr ? response->GetCode(XCode::Failure) : XCode::Failure;
	}

	XCode RpcService::Call(long long userId, const std::string& func, const Message& message)
	{
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(userId, func, &message);
        return response != nullptr ? response->GetCode(XCode::Failure) : XCode::Failure;
	}

	XCode RpcService::Call(long long userId, const std::string& func, std::shared_ptr<Message> response)
	{
        std::shared_ptr<Rpc::Packet> responsePackage = this->CallAwait(userId, func);
        if(responsePackage == nullptr)
        {
            return XCode::Failure;
        }
        XCode code = responsePackage->GetCode(XCode::Failure);
        if(code == XCode::Successful)
        {
            if(!responsePackage->ParseMessage(response.get()))
            {
                return XCode::ParseMessageError;
            }
            return XCode::Successful;
        }
        return code;
	}

	XCode RpcService::Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response)
	{
        std::shared_ptr<Rpc::Packet> responsePackage = this->CallAwait(userId, func, &message);
        if(responsePackage == nullptr)
        {
            return XCode::Failure;
        }
        XCode code = responsePackage->GetCode(XCode::Failure);
        if(code == XCode::Successful)
        {
            if(!responsePackage->ParseMessage(response.get()))
            {
                return XCode::ParseMessageError;
            }
            return XCode::Successful;
        }
        return code;
	}

    bool RpcService::StartSend(long long userId, const std::string &func, const Message *message)
    {
        assert(userId > 0);
        const std::string fullName = fmt::format("{0}.{1}", this->GetName(), func);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            LOG_ERROR("not find [" << fullName << "] config");
            return false;
        }
        if(!methodConfig->Request.empty())
        {
            if(message == nullptr)
            {
                LOG_ERROR("call [" << fullName << "] request is empty");
                return false;
            }
            if(message->GetTypeName() != methodConfig->Request)
            {
                LOG_ERROR("call [" << fullName << "] request type error");
                return false;
            }
        }
        std::shared_ptr<Rpc::Packet> request = std::make_shared<Rpc::Packet>();
        {
            request->WriteMessage(message);
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->GetHead().Add("id", userId);
            request->GetHead().Add("func", methodConfig->FullName);
        }
        std::string address;
#ifdef __INNER_MSG_FORWARD__
        LocationUnit * locationUnit = this->mLocationComponent->GetLocationUnit(userId);
        if(locationUnit != nullptr && locationUnit->Get(methodConfig->Service, address))
        {
            return this->mMessageComponent->Send(address, request);
        }
#endif
        this->mForwardComponent->GetLocation(userId, address);
        return this->mMessageComponent->Send(address, request);
    }

    std::shared_ptr<Rpc::Packet> RpcService::CallAwait(
        long long userId, const std::string &func, const Message *message)
    {
        assert(userId > 0);
        const std::string fullName = fmt::format("{0}.{1}", this->GetName(), func);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
        if(methodConfig == nullptr)
        {
            LOG_ERROR("not find [" << fullName << "] config");
            return nullptr;
        }

        if(!methodConfig->Request.empty())
        {
            if(message == nullptr)
            {
                LOG_ERROR("call [" << fullName << "] request is empty");
                return nullptr;
            }
            if(message->GetTypeName() != methodConfig->Request)
            {
                LOG_ERROR("call [" << fullName << "] request type error");
                return nullptr;
            }
        }

        std::shared_ptr<Rpc::Packet> request = std::make_shared<Rpc::Packet>();
        {
            request->WriteMessage(message);
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->GetHead().Add("func", func);
        }
        std::string address;
#ifdef __INNER_MSG_FORWARD__
        LocationUnit * locationUnit = this->mLocationComponent->GetLocationUnit(userId);
        if(locationUnit != nullptr && locationUnit->Get(methodConfig->Service, address))
        {
            return this->mMessageComponent->Call(address, request);
        }
#endif
        this->mForwardComponent->GetLocation(userId, address);
        return this->mMessageComponent->Call(address, request);
    }
}
