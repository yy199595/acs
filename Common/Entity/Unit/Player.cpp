//
// Created by leyi on 2023/5/15.
//

#include"Player.h"
#include"XCode/XCode.h"
#include"Entity/Unit/App.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/LocationComponent.h"
namespace Tendo
{
	Player::Player(long long id)
		: NetUnit(id) { }

	int Player::Send(const std::string& func)
	{
		std::string address;
		std::shared_ptr<Msg::Packet> message;
		int code = this->MakeRequest(func, nullptr, address, message);
		if(code != XCode::Successful)
		{
			return code;
		}
		if(! this->mInnerComponent->Send(address, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	int Player::Send(const std::string& func, const pb::Message& request)
	{
		std::string address;
		std::shared_ptr<Msg::Packet> message;
		int code = this->MakeRequest(func, &request, address, message);
		if(code != XCode::Successful)
		{
			return code;
		}
		if(! this->mInnerComponent->Send(address, message))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	int Player::Call(const std::string& func, const pb::Message& request)
	{
		std::string address;
		std::shared_ptr<Msg::Packet> message;
		int code = this->MakeRequest(func, &request, address, message);
		if(code != XCode::Successful)
		{
			return code;
		}
		std::shared_ptr<Msg::Packet> response = this->mInnerComponent->Call(address, message);
		if(response == nullptr)
		{
			return XCode::NetWorkError;
		}
		return response->GetCode();
	}

	int Player::Call(const std::string& func, const pb::Message& request, std::shared_ptr<pb::Message> result)
	{
		std::string address;
		std::shared_ptr<Msg::Packet> message;
		int code = this->MakeRequest(func, &request, address, message);
		if(code != XCode::Successful)
		{
			return code;
		}
		std::shared_ptr<Msg::Packet> response = this->mInnerComponent->Call(address, message);
		if(response == nullptr)
		{
			return XCode::NetWorkError;
		}
		code = response->GetCode();
		if(code == XCode::Successful)
		{
			const std::string & data = response->GetBody();
			if(!result->ParseFromArray(data.c_str(), data.size()))
			{
				return XCode::ParseMessageError;
			}
		}
		return code;
	}

	bool Player::GetAddr(const std::string& server, std::string& addr)
	{
		auto iter = this->mServerAddrs.find(server);
		if(iter == this->mServerAddrs.end())
		{
			return false;
		}
		int targetId = iter->second;
		const std::string listen("rpc");
		return this->mLocationComponent->GetServerAddress(targetId, listen, addr);
	}

	bool Player::GetAddr(const std::string& server, int& targetId)
	{
		auto iter = this->mServerAddrs.find(server);
		if(iter == this->mServerAddrs.end())
		{
			return false;
		}
		targetId = iter->second;
		return true;
	}

	void Player::AddAddr(const std::string& server, int id)
	{
		this->mServerAddrs[server] = id;
	}


	int Player::MakeRequest(const string& func, const pb::Message* message, std::string &addr, std::shared_ptr<Msg::Packet>& request)
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		if(!this->GetAddr(methodConfig->Server, addr))
		{
			return XCode::NotFoundPlayerRpcAddress;
		}
		request = std::make_shared<Msg::Packet>();
		{
			request->SetType(Msg::Type::Request);
			request->SetProto(Msg::Porto::Protobuf);
			if(!request->WriteMessage(message))
			{
				return XCode::SerializationFailure;
			}
			request->GetHead().Add("id", this->GetUnitId());
		}
		return XCode::Successful;
	}

	bool Player::DelAddr(const string& server)
	{
		auto iter = this->mServerAddrs.find(server);
		if(iter == this->mServerAddrs.end())
		{
			return false;
		}
		this->mServerAddrs.erase(iter);
		return true;
	}

	int Player::BroadCast(const std::string& func)
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			return 0;
		}
		int count = 0;
		std::string address;
		const std::string listen("rpc");
		for(const auto & info : this->mServerAddrs)
		{
			int targetId = info.second;
			if(this->mLocationComponent->GetServerAddress(targetId,listen, address))
			{
				std::shared_ptr<Msg::Packet> message;
				if (this->NewRequest(func, nullptr, message) == XCode::Successful)
				{
					if (this->mInnerComponent->Send(address, message))
					{
						count++;
					}
				}
			}
		}
		return count;
	}

	int Player::BroadCast(const std::string& func, const Message& request)
	{
		return 0;
	}


}