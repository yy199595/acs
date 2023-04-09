#include"RpcService.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Lua/LuaServiceMethod.h"
#include"Server/Config/ServiceConfig.h"
#include"Util/String/StringHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"
#ifdef __RPC_DEBUG_LOG__
#include<google/protobuf/util/json_util.h>
#endif
namespace Tendo
{
    RpcService::RpcService()
    {
        this->mNetComponent = nullptr;
        this->mLocationComponent = nullptr;
    }

	bool RpcService::LateAwake()
	{
        this->mNetComponent = this->GetComponent<InnerNetComponent>();
        this->mLocationComponent = this->GetComponent<NodeMgrComponent>();
		ClusterConfig::Inst()->GetServerName(this->GetName(), this->mCluster);
        if (!ServerConfig::Inst()->GetLocation("rpc", this->mLocationAddress))
        {
            LOG_WARN("not config rpc address");
        }
        return true;
	}
    bool RpcService::RandomAddress(std::string& address)
    {
        return this->mLocationComponent->GetServer(this->mCluster, address);
    }
}

namespace Tendo
{

	int RpcService::Send(const string& address, const string& func)
	{
		if(!this->StartSend(address, func, nullptr))
		{
			return XCode::Failure;
		}
		return XCode::Successful;
	}

	int RpcService::Send(const std::string& func, const Message& message)
	{
		std::vector<std::string> locations;		
		if(!this->mLocationComponent->GetServers(this->mCluster, locations))
		{
			LOG_ERROR(this->mCluster <<  " address list empty");
			return XCode::Failure;
		}

		for(const std::string & address : locations)
		{
            if(address != this->mLocationAddress &&
				!this->StartSend(address, func, &message))
            {
                LOG_ERROR("send to " << address << "failure");
            }
		}
		return XCode::Successful;
	}

    bool RpcService::StartSend(const std::string& address, const std::string& func, const Message* message)
    {
        const std::string name = fmt::format("{0}.{1}", this->GetName(), func);
        const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(name);
        if (methodConfig == nullptr)
        {
            LOG_ERROR("not find rpc method config " << name);
            return false;
        }
        std::shared_ptr<Rpc::Packet> request =
            std::make_shared<Rpc::Packet>();
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->WriteMessage(message);
            request->GetHead().Add("func", name);
        }
        return this->mNetComponent->Send(address, request);
    }

    std::shared_ptr<Rpc::Packet> RpcService::CallAwait(
        const std::string &address, const std::string &func, const Message *message)
	{
        const std::string name = fmt::format("{0}.{1}", this->GetName(), func);
        const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(name);
        if (methodConfig == nullptr)
        {
            LOG_ERROR("not find rpc method config " << name);
            return nullptr;
        }
        std::shared_ptr<Rpc::Packet> request =
            std::make_shared<Rpc::Packet>();
        {
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->WriteMessage(message);
            request->GetHead().Add("func", name);
        }
		return this->mNetComponent->Call(address, request);
	}

	int RpcService::Send(const std::string& address, const std::string& func, const Message& message)
    {
        if(!this->StartSend(address, func, &message))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

	int RpcService::Call(const std::string & address, const string& func)
    {
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(address, func);
        if (response == nullptr)
        {
            return XCode::NetWorkError;
        }
        return response->GetCode(XCode::Failure);
    }

	int RpcService::Call(const std::string & address, const string& func, const Message& message)
	{
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(address, func, &message);
		if(response == nullptr)
		{
			return XCode::NetWorkError;
		}
        return response->GetCode(XCode::Failure);
    }

	int RpcService::Call(const std::string & address, const string& func, std::shared_ptr<Message> response)
	{
		assert(response != nullptr);
		std::shared_ptr<Rpc::Packet> data = this->CallAwait(address, func, nullptr);
		if (data == nullptr)
		{
			return XCode::Failure;
		}
		if (data->GetCode() != XCode::Successful)
		{
			return data->GetCode();
		}
		if (!data->ParseMessage(response.get()))
		{
			return XCode::ParseMessageError;
		}
		return XCode::Successful;
	}


	int RpcService::Call(const std::string & address, const string& func, const Message& message,
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

namespace Tendo
{
    int RpcService::Send(long long userId, const std::string &func)
    {
        if(!this->StartSend(userId, func))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }

    int RpcService::Send(long long int userId, const string& func, const Message& message)
    {
        if(!this->StartSend(userId, func, &message))
        {
            return XCode::Failure;
        }
        return XCode::Successful;
    }


	int RpcService::Call(long long userId, const std::string& func)
	{
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(userId, func);
        return response != nullptr ? response->GetCode(XCode::Failure) : XCode::Failure;
	}

	int RpcService::Call(long long userId, const std::string& func, const Message& message)
	{
        std::shared_ptr<Rpc::Packet> response = this->CallAwait(userId, func, &message);
        return response != nullptr ? response->GetCode(XCode::Failure) : XCode::Failure;
	}

	int RpcService::Call(long long userId, const std::string& func, std::shared_ptr<Message> response)
	{
        std::shared_ptr<Rpc::Packet> responsePackage = this->CallAwait(userId, func);
        if(responsePackage == nullptr)
        {
            return XCode::Failure;
        }
        int code = responsePackage->GetCode(XCode::Failure);
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

	int RpcService::Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response)
	{
        std::shared_ptr<Rpc::Packet> responsePackage = this->CallAwait(userId, func, &message);
        if(responsePackage == nullptr)
        {
            return XCode::Failure;
        }
        int code = responsePackage->GetCode(XCode::Failure);
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

        std::string address;
		if(!this->mLocationComponent->GetServer(this->mCluster, userId, address))
        {
            return XCode::Failure;
        }
        std::shared_ptr<Rpc::Packet> request = std::make_shared<Rpc::Packet>();
        {
            request->WriteMessage(message);
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->GetHead().Add("id", userId);
            request->GetHead().Add("func", methodConfig->FullName);
        }
        return this->mNetComponent->Send(address, request);
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

        if(!methodConfig->Request.empty() && message != nullptr)
        {
            if(message->GetTypeName() != methodConfig->Request)
            {
                LOG_ERROR("call [" << fullName << "] request type error");
                return nullptr;
            }
        }

        std::string address;
        if(!this->mLocationComponent->GetServer(this->mCluster, userId, address))
        {
            return nullptr;
        }
        std::shared_ptr<Rpc::Packet> request = std::make_shared<Rpc::Packet>();
        {
            request->WriteMessage(message);
            request->SetType(Tcp::Type::Request);
            request->SetProto(Tcp::Porto::Protobuf);
            request->GetHead().Add("func", methodConfig->FullName);
        }
        return this->mNetComponent->Call(address, request);
    }

	int RpcService::Send(const std::string & address,
		const std::string & func, long long userId, const Message * message)
	{
		const std::string fullName = fmt::format("{0}.{1}", this->GetName(), func);
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
		if(methodConfig == nullptr)
		{
			LOG_ERROR("not find [" << fullName << "] config");
			return XCode::NotFoundRpcConfig;
		}

		if(!methodConfig->Request.empty() && message != nullptr)
		{
			if(message->GetTypeName() != methodConfig->Request)
			{
				LOG_ERROR("call [" << fullName << "] request type error");
				return XCode::CallArgsError;
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
		return this->mNetComponent->Send(address, request);
	}
}
