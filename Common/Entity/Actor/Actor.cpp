//
// Created by yjz on 2023/5/17.
//

#include"Actor.h"
#include"XCode/XCode.h"
#include"Entity/Unit/App.h"
#include"Rpc/Component/InnerNetComponent.h"
#include<google/protobuf/util/json_util.h>
namespace pb_util = google::protobuf::util;
namespace Tendo
{
	Actor::Actor(long long id, const std::string& addr)
		: Unit(id), mAddr(addr)
	{
		this->mNetComponent = nullptr;
	}

	bool Actor::LateAwake()
	{
		this->mNetComponent = App::Inst()->GetComponent<InnerNetComponent>();
		LOG_CHECK_RET_FALSE(this->mNetComponent);
		return true;
	}

	int Actor::Send(const std::string& func)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
		}
		if(!this->mNetComponent->Send(this->mAddr, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}

	int Actor::Send(const std::string& func, const pb::Message& request)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
			message->SetProto(Msg::Porto::Protobuf);
			if(!message->WriteMessage(&request))
			{
				return XCode::SerializationFailure;
			}
		}
		if(!this->mNetComponent->Send(this->mAddr, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}

	int Actor::Call(const std::string& func)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
		}
		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Actor::Call(const std::string& func, const pb::Message& request)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
			message->SetProto(Msg::Porto::Protobuf);
			if(!message->WriteMessage(&request))
			{
				return XCode::SerializationFailure;
			}
		}
		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Actor::Call(const std::string& func, std::shared_ptr<pb::Message> response)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
		}
		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		int code = result != nullptr ? result->GetCode() : XCode::NetWorkError;
		if(code == XCode::Successful)
		{
			if(!result->ParseMessage(response.get()))
			{
				return XCode::ParseMessageError;
			}
		}
		return code;
	}

	int Actor::Call(const std::string& func, const pb::Message& request, std::shared_ptr<pb::Message> response)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
			message->SetProto(Msg::Porto::Protobuf);
			if(!message->WriteMessage(&request))
			{
				return XCode::SerializationFailure;
			}
		}
		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		int code = result != nullptr ? result->GetCode() : XCode::NetWorkError;
		if(code == XCode::Successful)
		{
			if(!result->ParseMessage(response.get()))
			{
				return XCode::ParseMessageError;
			}
		}
		return code;
	}
}