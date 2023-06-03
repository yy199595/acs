//
// Created by leyi on 2023/5/23.
//

#include"ServerActor.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Component/InnerNetComponent.h"
namespace Tendo
{
	ServerActor::ServerActor(int id, const std::string & name)
		: Actor(id, name), mRpc("rpc")
	{

	}

	bool ServerActor::OnInit()
	{
		if(!this->GetListen(this->mRpc, this->mRpcAddress))
		{
			LOG_ERROR("not rpc address " << this->Name());
			return false;
		}
		return true;
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

	void ServerActor::AddListen(const std::string& name, const std::string& addr)
	{
		if (addr.empty() || name.empty())
		{
			return;
		}
		this->mListens[name] = addr;
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

	int ServerActor::Send(const std::shared_ptr<Msg::Packet>& message)
	{
		if(!this->mNetComponent->Send(this->mRpcAddress, message))
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