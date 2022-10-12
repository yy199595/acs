#include"Service.h"
#include"App/App.h"
#include"Lua/LuaServiceMethod.h"
#include"Config/ServiceConfig.h"
#include"String/StringHelper.h"
#include"Async/RpcTaskSource.h"
#include"Lua/LuaService.h"
#include"Component/InnerNetComponent.h"
#include"Component/InnerNetMessageComponent.h"
#ifdef __RPC_DEBUG_LOG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Sentry
{
	Service::Service()
	{
		this->mConfig = nullptr;
	}
	bool Service::LateAwake()
	{
		assert(this->mConfig);
		this->mClientComponent = this->GetComponent<InnerNetComponent>();
        this->mMessageComponent = this->GetComponent<InnerNetMessageComponent>();
        return this->GetConfig().GetListener("rpc", this->mLocalAddress);
	}

	void Service::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<Service>();
		luaRegister.PushExtensionFunction("Call", Lua::Service::Call);
        luaRegister.PushExtensionFunction("AllotLocation", Lua::Service::AllotLocation);
	}

    const RpcMethodConfig *Service::GetMethodConfig(const std::string &method) const
    {
        return this->mConfig->GetConfig(method);
    }

}

namespace Sentry
{
	XCode Service::Send(const std::string& func, const Message& message)
	{
        this->mServiceHosts.clear();
        if(!this->GetHosts(this->mServiceHosts))
        {
            LOG_ERROR("service address list empty");
            return XCode::CallServiceNotFound;
        }
		for(const std::string & address : this->mServiceHosts)
		{
            if(!this->StartSend(address, func, 0, &message))
            {
                LOG_ERROR("send to " << address << "failure");
            }
		}
		return XCode::Successful;
	}

    bool Service::StartSend(const std::string &address,
                            const std::string &func, long long userId, const Message *message)
    {
        std::shared_ptr<Rpc::Data> request
            = std::make_shared<Rpc::Data>();

        request->SetType(Tcp::Type::Request);
        request->SetProto(Tcp::Porto::Protobuf);
        request->GetHead().Add("func", func);
        if(userId != 0)
        {
            request->GetHead().Add("id", userId);
        }
        request->WriteMessage(message);
        return this->mMessageComponent->Send(address, request);
    }

    std::shared_ptr<Rpc::Data> Service::StartCall(
        const std::string &address, const std::string &func, long long userId, const Message *message)
    {
        const RpcMethodConfig * methodConfig  = this->mConfig->GetConfig(func);
        if(methodConfig == nullptr)
        {
            return nullptr;
        }
        std::shared_ptr<Rpc::Data> request =
            Rpc::Data::New(Tcp::Type::Request, Tcp::Porto::Protobuf);
        request->GetHead().Add("func", func);
        if(userId != 0)
        {
            request->GetHead().Add("id", userId);
        }
        if(!methodConfig->Request.empty() && message == nullptr)
        {
            CONSOLE_LOG_ERROR("call " << func << " request error");
            return nullptr;
        }
        request->WriteMessage(message);
        return this->mMessageComponent->Call(address, request);
    }

	XCode Service::Send(long long int userId, const string& func, const Message& message)
	{
        return XCode::Successful;
	}

	XCode Service::Send(const std::string& address, const std::string& func, const Message& message)
	{
        std::shared_ptr<Rpc::Data> request
            = std::make_shared<Rpc::Data>();

        request->WriteMessage(&message);
        request->SetType(Tcp::Type::Request);
        request->SetProto(Tcp::Porto::Protobuf);
        request->GetHead().Add("func", func);
        return this->mMessageComponent->Send(address, request) ? XCode::Successful : XCode::NetWorkError;
	}

	XCode Service::Call(const std::string & address, const string& func)
    {
        std::shared_ptr<Rpc::Data> response = this->StartCall(address, func, 0, nullptr);
        if (response == nullptr)
        {
            return XCode::NetWorkError;
        }
        return response->GetCode(XCode::Failure);
    }

	XCode Service::Call(const std::string & address, const string& func, const Message& message)
	{
        std::shared_ptr<Rpc::Data> response = this->StartCall(address, func, 0, &message);
		if(response == nullptr)
		{
			return XCode::NetWorkError;
		}
        return response->GetCode(XCode::Failure);
    }

	XCode Service::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
        std::shared_ptr<Rpc::Data> data = this->StartCall(address, func, 0, nullptr);
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


	XCode Service::Call(const std::string & address, const string& func, const Message& message,
			std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
        std::shared_ptr<Rpc::Data> data =
            this->StartCall(address, func, 0, &message);
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
	XCode Service::Call(long long userId, const std::string& func)
	{
        return XCode::Successful;
	}

	XCode Service::Call(long long userId, const std::string& func, const Message& message)
	{

        return XCode::Successful;
	}

	XCode Service::Call(long long userId, const std::string& func, std::shared_ptr<Message> response)
	{
        return XCode::Successful;
	}

	XCode Service::Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response)
	{

        return XCode::Successful;
	}

	bool Service::LoadConfig(const rapidjson::Value& json)
	{
		if(this->mConfig == nullptr)
		{
			this->mConfig = new RpcServiceConfig(this->GetName());
		}
		return this->mConfig->OnLoadConfig(json);
	}
}
