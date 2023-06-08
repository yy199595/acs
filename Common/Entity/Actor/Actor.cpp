//
// Created by yjz on 2023/5/17.
//
#include"App.h"
#include"Actor.h"
#include"XCode/XCode.h"
#include"Rpc/Client/Message.h"
#include"Util/Time/TimeHelper.h"
#include"Lua/Engine/LuaParameter.h"
#include"Proto/Include/Message.h"
#include"Util/Json/Lua/Json.h"
#include"Server/Config/ServiceConfig.h"
#include"Proto/Component/ProtoComponent.h"
#include"Router/Component/RouterComponent.h"
namespace Tendo
{
	Actor::Actor(long long id, const std::string & name)
		: Entity(id), mName(name)
	{
		this->mRouterComponent = nullptr;
		this->mLastTime = Helper::Time::NowSecTime();
	}

	bool Actor::LateAwake()
	{
		this->mRouterComponent = App::Inst()->GetComponent<RouterComponent>();
		LOG_CHECK_RET_FALSE(this->mRouterComponent != nullptr && this->OnInit());
		return true;
	}

	int Actor::Send(const std::string& func)
	{
		std::string addr;
		if(!this->GetAddress(func, addr))
		{
			return XCode::NotFoundServerRpcAddress;
		}
		this->mLastTime = Helper::Time::NowSecTime();
		const std::shared_ptr<Msg::Packet> message = this->Make(func, nullptr);
		if(message == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->mRouterComponent->Send(addr, message) ? XCode::Successful : XCode::SendMessageFail;
	}

	int Actor::Send(const std::string& func, const pb::Message& request)
	{
		std::string addr;
		const int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		this->mLastTime = Helper::Time::NowSecTime();
		const std::shared_ptr<Msg::Packet> message = this->Make(func, &request);
		if(message == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		return this->mRouterComponent->Send(addr, message) ? XCode::Successful : XCode::SendMessageFail;
	}

	int Actor::Call(const std::string& func)
	{
		std::string addr;
		const int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		this->mLastTime = Helper::Time::NowSecTime();
		const std::shared_ptr<Msg::Packet> message = this->Make(func, nullptr);
		if(message == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		const std::shared_ptr<Msg::Packet> result = this->mRouterComponent->Call(addr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	std::shared_ptr<Msg::Packet> Actor::Make(const std::string& func, const pb::Message* request) const
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			LOG_ERROR("not rpc config : [" << func << "]");
			return nullptr;
		}
		std::shared_ptr<Msg::Packet> message = std::make_shared<Msg::Packet>();
		{
			message->SetNet(methodConfig->Net);
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
			message->GetHead().Add("id", this->GetEntityId());
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
		const int code = this->GetAddress(func, addr);
		if(code != XCode::Successful)
		{
			LOG_ERROR("call " << func << " code =" << code);
			return code;
		}
		this->mLastTime = Helper::Time::NowSecTime();
		const std::shared_ptr<Msg::Packet> message = this->Make(func, &request);
		if(message == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		const std::shared_ptr<Msg::Packet> result = this->mRouterComponent->Call(addr, message);
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
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
		this->mLastTime = Helper::Time::NowSecTime();
		const std::shared_ptr<Msg::Packet> message = this->Make(func, nullptr);
		if(message == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		const std::shared_ptr<Msg::Packet> result =
				this->mRouterComponent->Call(addr, message);
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
		this->mLastTime = Helper::Time::NowSecTime();
		const std::shared_ptr<Msg::Packet> message = this->Make(func, &request);
		if(message == nullptr)
		{
			return XCode::CallFunctionNotExist;
		}
		const std::shared_ptr<Msg::Packet> result =
				this->mRouterComponent->Call(addr, message);
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

	int Actor::MakeMessage(lua_State* lua, int idx,
		const std::string& func, std::shared_ptr<Msg::Packet>& message) const
	{
		App* app = App::Inst();
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if (methodConfig == nullptr)
		{
			LOG_ERROR("not found rpc config" << func);
			return XCode::NotFoundRpcConfig;
		}

		if (lua_istable(lua, idx))
		{
			if (!methodConfig->Request.empty())
			{
				std::shared_ptr<pb::Message> request;
				ProtoComponent* protoComponent = app->GetProto();
				const std::string& name = methodConfig->Request;
				if (!protoComponent->Read(lua, name, idx, request))
				{
					return XCode::CreateProtoFailure;
				}
				message = this->Make(func, request.get());
				return XCode::Successful;
			}
			message = this->Make(func, nullptr);
			message->SetProto(Msg::Porto::Json);
			message->SetNet(methodConfig->Net);
			Lua::RapidJson::Read(lua, idx, message->Body());
			return XCode::Successful;
		}
		if (lua_isstring(lua, idx))
		{
			size_t count = 0;
			message = this->Make(func, nullptr);
			const char* str = lua_tolstring(lua, idx, &count);
			message->Body()->append(str, count);
			message->SetProto(Msg::Porto::String);
			return XCode::Successful;
		}
		return XCode::CallArgsError;
	}

	int Actor::LuaCall(lua_State* lua, const std::string & func, const std::shared_ptr<Msg::Packet> & message)
	{
		std::string address;
		if(!this->GetAddress(func, address))
		{
			lua_pushinteger(lua, XCode::NotFoundPlayerRpcAddress);
			return 1;
		}
		this->mLastTime = Helper::Time::NowSecTime();
		return this->mRouterComponent->LuaCall(lua, address, message);
	}

	int Actor::LuaSend(lua_State* lua, const string& func, const std::shared_ptr<Msg::Packet>& message)
	{
		std::string address;
		int code = XCode::Successful;
		do
		{
			if(!this->GetAddress(func, address))
			{
				code = XCode::NotFoundPlayerRpcAddress;
				break;
			}
			this->mLastTime = Helper::Time::NowSecTime();
			if(!this->mRouterComponent->Send(address, message))
			{
				code = XCode::SendMessageFail;
				break;
			}
		}
		while(false);
		lua_pushinteger(lua, code);
		return 1;
	}
}