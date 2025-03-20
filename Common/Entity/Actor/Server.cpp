//
// Created by leyi on 2023/5/23.
//

#include"Server.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Http/Common/HttpRequest.h"
#include"Http/Common/HttpResponse.h"
#include"Util/Tools/String.h"
#include"Http/Component/HttpComponent.h"
#include"Router/Component/RouterComponent.h"
namespace acs
{
	Server::Server(int id, const std::string & name)
		: Actor(id, name), mRpc("rpc")
	{
		this->mAppId = 0;
	}

	bool Server::OnInit()
	{
		if(!this->GetListen(this->mRpc, this->mRpcAddress))
		{
			LOG_WARN("not rpc address:{} id:{}", this->Name(), this->GetId());
		}
		this->mAppId = App::Inst()->GetId();
		return true;
	}

	bool Server::GetAddress(const rpc::Message & request, int & id) const
	{
		id = this->GetSrvId();
		return true;
	}

	int Server::DisConnect()
	{
		std::unique_ptr<rpc::Message> message = std::make_unique<rpc::Message>();
		{
			message->SetNet(rpc::Net::Tcp);
			message->SetType(rpc::Type::Logout);
		}
		return this->mRouter->Send(this->GetSrvId(), std::move(message));
	}

	bool Server::AddListen(const std::string& name, const std::string& addr)
	{
		LOG_CHECK_RET_FALSE(!addr.empty() && !name.empty());

		std::string ip, net;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(addr, ip, port))
		{
			if(!help::Str::SplitAddr(addr, net, ip, port))
			{
				LOG_ERROR("add {} {} {} fail", this->Name(), name, addr);
				return false;
			}
		}
		auto iter = this->mListens.find(name);
		std::string data = fmt::format("{0}:{1}", ip, port);
		if(iter != this->mListens.end() && iter->second == data)
		{
			return false;
		}
		this->mListens[name] = data;
		return true;
	}

	bool Server::GetListen(const std::string& name, std::string& addr) const
	{
		auto iter = this->mListens.find(name);
		if(iter == this->mListens.end())
		{
			LOG_WARN("{} not {} address", this->Name(), name);
			return false;
		}
		addr = iter->second;
		return true;
	}

	int Server::SendMsg(std::unique_ptr<rpc::Message> message)
	{
		return this->mRouter->Send(this->GetSrvId(), std::move(message));
	}

	void Server::Encode(json::w::Value & jsonWriter)
	{
		jsonWriter.Add("name", this->Name());
		jsonWriter.Add("id", this->GetSrvId());
		std::unique_ptr<json::w::Value> data = jsonWriter.AddObject("listen");
		for(auto iter = this->mListens.begin(); iter != this->mListens.end(); iter++)
		{
			data->Add(iter->first.c_str(), iter->second);
		}
	}

	bool Server::Decode(json::r::Value& document)
	{
		long long id = 0;
		if(!document.Get("id", id) || !this->Equal(id))
		{
			return false;
		}
		std::unique_ptr<json::r::Value> jsonValue;
		if(!document.Get("listen", jsonValue))
		{
			return false;
		}
		for(const char * key : jsonValue->GetAllKey())
		{
			std::string value;
			if(!jsonValue->Get(key, value))
			{
				return false;
			}
			this->mListens[key] = value;
			CONSOLE_LOG_DEBUG("[{}:{}] add listen => {}://{}", this->Name(), id, key, value)
		}
		return true;
	}

	int Server::Make(const std::string& func, std::unique_ptr<rpc::Message>& message) const
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			LOG_ERROR("not rpc config {}", func);
			return XCode::NotFoundRpcConfig;
		}
		message = std::make_unique<rpc::Message>();
		{
			message->SetNet(methodConfig->net);
			message->SetType(rpc::Type::Request);
			message->SetProto(methodConfig->proto);
			message->GetHead().Add(rpc::Header::func, func);
			message->GetHead().Add(rpc::Header::app_id, this->mAppId);
		}
		message->SetSockId(this->GetSrvId());
		message->SetTimeout(methodConfig->timeout);
		return XCode::Ok;
	}
}