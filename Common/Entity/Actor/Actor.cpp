//
// Created by yjz on 2023/5/17.
//

#include"Actor.h"
#include"XCode/XCode.h"
#include"App.h"
#include"Rpc/Component/InnerNetComponent.h"
#include<google/protobuf/util/json_util.h>
#include"Server/Config/ServiceConfig.h"
namespace pb_util = google::protobuf::util;
namespace Tendo
{
	Actor::Actor(long long id, std::string  addr)
		: Unit(id), mAddr(std::move(addr))
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
		std::shared_ptr<Msg::Packet> message = this->Make(func, nullptr);
		if(!this->mNetComponent->Send(this->mAddr, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}

	int Actor::Send(const std::string& func, const pb::Message& request)
	{
		std::string addr;
		int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		std::shared_ptr<Msg::Packet> message = this->Make(func, &request);
		if(!this->mNetComponent->Send(this->mAddr, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}

	int Actor::Call(const std::string& func)
	{
		std::string addr;
		int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
		}

		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(addr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	std::shared_ptr<Msg::Packet> Actor::Make(const std::string& func, const pb::Message* request)
	{
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
			message->GetHead().Add("id", this->GetUnitId());
			if(request != nullptr)
			{
				message->SetProto(Msg::Porto::Protobuf);
				message->WriteMessage(request);
			}
		}
		return message;
	}

	int Actor::Call(const std::string& func, const pb::Message& request)
	{
		std::string addr;
		int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		std::shared_ptr<Msg::Packet> message = this->Make(func, &request);

		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Actor::GetAddress(const std::string& func, std::string& addr)
	{
		addr = this->mAddr;
		return XCode::Successful;
	}

	int Actor::Call(const std::string& func, std::shared_ptr<pb::Message> response)
	{
		std::string addr;
		int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		std::shared_ptr<Msg::Packet> message = this->Make(func, nullptr);

		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		code = result != nullptr ? result->GetCode() : XCode::NetWorkError;
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
		std::string addr;
		int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		std::shared_ptr<Msg::Packet> message = this->Make(func, &request);

		std::shared_ptr<Msg::Packet> result =
			this->mNetComponent->Call(this->mAddr, message);
		code = result != nullptr ? result->GetCode() : XCode::NetWorkError;
		if(code == XCode::Successful)
		{
			if(!result->ParseMessage(response.get()))
			{
				return XCode::ParseMessageError;
			}
		}
		return code;
	}

	int Actor::Send(const std::shared_ptr<Msg::Packet>& message)
	{
		if(!this->mNetComponent->Send(this->mAddr, message))
		{
			return XCode::SendMessageFail;
		}
		return XCode::Successful;
	}
}