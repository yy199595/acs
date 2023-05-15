//
// Created by leyi on 2023/5/15.
//

#include "Server.h"
#include "XCode/XCode.h"
#include "Rpc/Component/LocationComponent.h"
#include "Rpc/Component/InnerNetComponent.h"
namespace Tendo
{
	int Server::Send(const std::string& addr, const std::string& func)
	{
		std::shared_ptr<Msg::Packet> message;
		if(this->NewRequest(func, nullptr, message) != XCode::Successful)
		{
			return XCode::NotFoundRpcConfig;
		}
		this->mInnerComponent->Send(addr, message);
		return XCode::Successful;
	}

	int Server::Send(const std::string& addr, const std::string& func, const pb::Message& request)
	{
		std::shared_ptr<Msg::Packet> message;
		if(this->NewRequest(func, &request, message) != XCode::Successful)
		{
			return XCode::NotFoundRpcConfig;
		}
		this->mInnerComponent->Send(addr, message);
		return XCode::Successful;
	}

	int Server::Call(const std::string& addr, const std::string& func)
	{
		std::shared_ptr<Msg::Packet> message;
		if(this->NewRequest(func, nullptr, message) != XCode::Successful)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<Msg::Packet> result = this->mInnerComponent->Call(addr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Server::Call(const std::string& addr, const std::string& func, const pb::Message& request)
	{
		std::shared_ptr<Msg::Packet> message;
		if(this->NewRequest(func, &request, message) != XCode::Successful)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<Msg::Packet> result = this->mInnerComponent->Call(addr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Server::Call(const std::string& addr, const std::string& func, std::shared_ptr<pb::Message> response)
	{
		std::shared_ptr<Msg::Packet> message;
		if(this->NewRequest(func, nullptr, message) != XCode::Successful)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<Msg::Packet> result = this->mInnerComponent->Call(addr, message);
		if(result == nullptr)
		{
			return XCode::NetWorkError;
		}
		if(result->GetCode() == XCode::Successful)
		{
			const std::string & data = result->GetBody();
			if(!response->ParseFromArray(data.c_str(), data.size()))
			{
				return XCode::ParseMessageError;
			}
			return XCode::Successful;
		}
		return result->GetCode();
	}

	int Server::Call(const std::string& addr, const std::string& func,
			const pb::Message& request, std::shared_ptr<pb::Message> response)
	{
		std::shared_ptr<Msg::Packet> message;
		if(this->NewRequest(func, &request, message) != XCode::Successful)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::shared_ptr<Msg::Packet> result = this->mInnerComponent->Call(addr, message);
		if(result == nullptr)
		{
			return XCode::NetWorkError;
		}
		if(result->GetCode() == XCode::Successful)
		{
			const std::string & data = result->GetBody();
			if(!response->ParseFromArray(data.c_str(), data.size()))
			{
				return XCode::ParseMessageError;
			}
			return XCode::Successful;
		}
		return result->GetCode();
	}

}

namespace Tendo
{
	bool Server::GetAddr(const std::string& server, int& targetId)
	{
		if(this->mLocationComponent == nullptr)
		{
			return false;
		}
		targetId = this->mLocationComponent->RangeServer(server);
		return true;
	}

	bool Server::GetAddr(const std::string& server, std::string& addr)
	{
		if(this->mLocationComponent == nullptr)
		{
			return false;
		}
		const std::string listen("rpc");
		int targetId = this->mLocationComponent->RangeServer(server);
		return this->mLocationComponent->GetServerAddress(targetId, listen, addr);
	}

	int Server::Send(const string& addr, const std::shared_ptr<Msg::Packet>& message)
	{
		return this->mInnerComponent->Send(addr, message);
	}
}