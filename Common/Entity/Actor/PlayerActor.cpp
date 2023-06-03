//
// Created by leyi on 2023/5/15.
//

#include"PlayerActor.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/Define.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
namespace Tendo
{
	PlayerActor::PlayerActor(long long playerId, int gateId)
		: Actor(playerId, "Player")
	{
		this->mGateId = gateId;
		this->mActorComponent = App::Inst()->ActorMgr();
	}

	bool PlayerActor::OnInit()
	{
		if(!this->mActorComponent->GetServer(this->mGateId))
		{
			LOG_ERROR("not find gate actor : " << this->mGateId);
			return false;
		}
		return true;
	}

	bool PlayerActor::DelAddr(const std::string& server)
	{
		auto iter = this->mServerAddrs.find(server);
		if(iter == this->mServerAddrs.end())
		{
			return false;
		}
		this->mServerAddrs.erase(iter);
		return true;
	}

	void PlayerActor::AddAddr(const std::string& server, int id)
	{
		if(server.empty() || id == 0)
		{
			return;
		}
		this->mServerAddrs[server] = id;
	}

	void PlayerActor::GetActors(std::vector<int>& actors) const
	{
		actors.reserve(this->mServerAddrs.size());
		auto iter = this->mServerAddrs.begin();
		for(; iter != this->mServerAddrs.end(); iter++)
		{
			actors.emplace_back(iter->second);
		}
	}

	int PlayerActor::SendToClient(const std::string& func)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Client);
			message->GetHead().Add("func", func);
			message->GetHead().Add("id", this->GetActorId());
		}
		return this->SendToClient(message);
	}

	int PlayerActor::SendToClient(const std::string& func, const pb::Message& request)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Client);
			message->SetProto(Msg::Porto::Protobuf);
			if(!message->WriteMessage(&request))
			{
				return XCode::SerializationFailure;
			}
			message->GetHead().Add("func", func);
			message->GetHead().Add("id", this->GetActorId());
			message->GetHead().Add("pb", request.GetTypeName());
		}
		return this->SendToClient(message);
	}

	int PlayerActor::GetAddress(const std::string& func, std::string& addr)
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		auto iter = this->mServerAddrs.find(methodConfig->Server);
		if(iter != this->mServerAddrs.end())
		{
			long long actorId = iter->second;
			Actor * targetActor = this->mActorComponent->GetActor(actorId);
			if(targetActor != nullptr && targetActor->GetAddress(func, addr))
			{
				return XCode::Successful;
			}
		}
		return XCode::NotFoundActor;
	}

	int PlayerActor::SendToClient(const std::shared_ptr<Msg::Packet>& message)
	{
		ServerActor * gateActor = this->mActorComponent->GetServer(this->mGateId);
		if(gateActor == nullptr)
		{
			return XCode::NotFoundActor;
		}
		return gateActor->Send(message);
	}
	void PlayerActor::OnRegister(std::string& json)
	{
		Json::Writer jsonWriter;
		jsonWriter.Add("id").Add(this->GetActorId());
		jsonWriter.BeginObject("addr");
		auto iter = this->mServerAddrs.begin();
		for(; iter != this->mServerAddrs.end(); iter++)
		{
			const std::string & name = iter->first;
			jsonWriter.Add(name).Add(iter->second);
		}
		jsonWriter.EndObject();
		jsonWriter.Add("time").Add(Helper::Time::NowSecTime());
		jsonWriter.WriterStream(&json);
	}
}