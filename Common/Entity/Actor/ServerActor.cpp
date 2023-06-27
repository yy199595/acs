//
// Created by leyi on 2023/5/23.
//

#include"ServerActor.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Util/Json/JsonWriter.h"
#include"Util/String/StringHelper.h"
#include"Router/Component/RouterComponent.h"
namespace Tendo
{
	ServerActor::ServerActor(int id, const std::string & name)
		: Actor(id, name), mRpc("rpc")
	{
		this->mServerId = 0;
	}

	bool ServerActor::OnInit()
	{
		if(!this->GetListen(this->mRpc, this->mRpcAddress))
		{
			LOG_WARN("not rpc address " << this->Name());
		}
		this->mServerId = App::Inst()->GetActorId();
		return true;
	}

	void ServerActor::OnWriteRpcHead(const std::string& func, Msg::Head& head) const
	{
		head.Add("id", this->mServerId);
	}

	int ServerActor::GetAddress(const std::string& func, std::string& addr)
	{
		auto iter = this->mListens.find(this->mRpc);
		if(iter == this->mListens.end())
		{
			return XCode::AddressAllotFailure;
		}
		addr = iter->second;
		return XCode::Successful;
	}

	bool ServerActor::AddListen(const std::string& name, const std::string& addr)
	{
		if (addr.empty() || name.empty())
		{
			return false;
		}
		std::string ip, net;
		unsigned short port = 0;
		if(Helper::Str::SplitAddr(addr, ip, port))
		{
			this->mListens[name] = addr;
			return true;
		}
		if(Helper::Str::SplitAddr(addr, net, ip, port))
		{
			this->mListens[name] = fmt::format("{0}:{1}", ip, port);
			return true;
		}
		LOG_FMT_ERR("add {0} {1} [{2}] fail", this->Name(), name, addr);
		return true;
	}

	bool ServerActor::GetListen(const std::string& name, std::string& addr)
	{
		auto iter = this->mListens.find(name);
		if(iter == this->mListens.end())
		{
			return false;
		}
		addr = iter->second;
		return true;
	}

	bool ServerActor::GetListen(int net, std::string& address)
	{
		switch(net)
		{
			case Msg::Net::Tcp:
				return this->GetListen("rpc", address);
			case Msg::Net::Http:
				return this->GetListen("http", address);
			case Msg::Net::Redis:
				return this->GetListen("redis", address);
		}
		return false;
	}

	int ServerActor::Send(const std::shared_ptr<Msg::Packet>& message)
	{
		std::string address;
		if(!this->GetListen(message->GetNet(), address))
		{
			return XCode::NotFoundActorAddress;
		}
		if(!this->mRouterComponent->Send(address, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}
	void ServerActor::OnRegister(std::string * json)
	{
		Json::Writer jsonWriter;
		auto iter = this->mListens.begin();
		jsonWriter.Add("name").Add(this->Name());
		jsonWriter.Add("id").Add(this->GetActorId());
		jsonWriter.BeginObject("listen");
		for(; iter != this->mListens.end(); iter++)
		{
			jsonWriter.Add(iter->first).Add(iter->second);
		}
		jsonWriter.EndObject();
		jsonWriter.Add("time").Add(Helper::Time::NowSecTime());
		jsonWriter.WriterStream(json);
	}
}