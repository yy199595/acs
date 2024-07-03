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
#include"Rpc/Config/ServiceConfig.h"
#include"Yyjson/Lua/ljson.h"
#include"Proto/Component/ProtoComponent.h"
#include"Router/Component/RouterComponent.h"
namespace joke
{
	Actor::Actor(long long id, std::string  name)
		: Entity(id), mName(std::move(name))
	{
		this->mProto = nullptr;
		this->mRouterComponent = nullptr;
		this->mLastTime = help::Time::NowSec();
	}

	bool Actor::LateAwake()
	{
		this->mProto = App::Inst()->GetProto();
		this->mRouterComponent = App::Inst()->GetComponent<RouterComponent>();
		LOG_CHECK_RET_FALSE(this->mRouterComponent != nullptr && this->OnInit());
		return true;
	}

	int Actor::Send(const std::string& func)
	{
		std::unique_ptr<rpc::Packet> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		this->mLastTime = help::Time::NowSec();
		return this->mRouterComponent->Send(id, std::move(message));
	}

	int Actor::Send(const std::string& func, const pb::Message& request)
	{
		std::unique_ptr<rpc::Packet> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		if(!message->WriteMessage(rpc::Porto::Protobuf, &request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		this->mLastTime = help::Time::NowSec();
		return this->mRouterComponent->Send(id, std::move(message));
	}

	int Actor::Call(const std::string& func)
	{
		std::unique_ptr<rpc::Packet> message ;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		this->mLastTime = help::Time::NowSec();
		rpc::Packet * result = this->mRouterComponent->Call(id, std::move(message));
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}
	
	int Actor::Call(const std::string& func, const pb::Message& request)
	{
		std::unique_ptr<rpc::Packet> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return XCode::MakeTcpRequestFailure;
		}
		if(!message->WriteMessage(rpc::Porto::Protobuf, &request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		this->mLastTime = help::Time::NowSec();
		const rpc::Packet * result = this->mRouterComponent->Call(id, std::move(message));
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Actor::Call(const std::string& func, pb::Message * response)
	{
		std::unique_ptr<rpc::Packet> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		rpc::Packet * result = this->mRouterComponent->Call(id, std::move(message));
		if(result == nullptr)
		{
			return XCode::NetWorkError;
		}
		if(result->GetCode() == XCode::Ok)
		{
			if(!result->ParseMessage(response))
			{
				return XCode::ParseMessageError;
			}
		}
		return code;
	}

	int Actor::Call(std::unique_ptr<rpc::Packet> message)
	{
		int id = 0;
		if(!this->GetAddress(*message, id))
		{
			return XCode::NotFoundActorAddress;
		}
		rpc::Packet * result = this->mRouterComponent->Call(id, std::move(message));
		if(result == nullptr)
		{
			return XCode::NetWorkError;
		}
		return result->GetCode();
	}

	int Actor::Call(const std::string& func, const pb::Message& request, pb::Message * response)
	{
		std::unique_ptr<rpc::Packet> message;
		int code = this->Make(func, message);
		if (code != XCode::Ok)
		{
			return code;
		}
		if(!message->WriteMessage(rpc::Porto::Protobuf, &request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		rpc::Packet* result = this->mRouterComponent->Call(id, std::move(message));
		if (result == nullptr)
		{
			return XCode::NetWorkError;
		}
		code = result->GetCode();
		if (code == XCode::Ok && !result->ParseMessage(response))
		{
			return XCode::ParseMessageError;
		}
		return code;
	}

	int Actor::MakeMessage(lua_State* lua, int idx,
		const std::string& func, std::unique_ptr<rpc::Packet> & message) const
	{
		const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if (methodConfig == nullptr)
		{
			LOG_ERROR("call {} fail not rpc config", func);
			return XCode::NotFoundRpcConfig;
		}
		int code = this->Make(func, message);
		if (code != XCode::Ok)
		{
			return code;
		}
		message->SetProto(rpc::Type::None);
		switch (lua_type(lua, idx))
		{
			case LUA_TNIL:
				message->SetProto(rpc::Porto::None);
				return XCode::Ok;
			case LUA_TSTRING:
			{
				size_t count = 0;
				const char* str = lua_tolstring(lua, idx, &count);
				{
					message->Body()->append(str, count);
					message->SetProto(rpc::Porto::String);
					if (!methodConfig->Response.empty())
					{
						message->GetHead().Add("res", methodConfig->Response);
					}
				}
				return XCode::Ok;
			}
			case LUA_TTABLE:
			{
				if (!methodConfig->Request.empty())
				{
					message->SetProto(rpc::Porto::Protobuf);
					const std::string& name = methodConfig->Request;
					pb::Message* request = this->mProto->Read(lua, name, idx);
					if (request == nullptr)
					{
						LOG_ERROR("call {} fail request : {}", func, name);
						return XCode::CreateProtoFailure;
					}
					if (!message->WriteMessage(request))
					{
						return XCode::SerializationFailure;
					}
					message->TempHead().Add("res", methodConfig->Response);

					return XCode::Ok;
				}
				message->SetProto(rpc::Porto::Json);
				if (!methodConfig->Response.empty())
				{
					message->GetHead().Add("res", methodConfig->Response);
				}
				lua::yyjson::read(lua, idx, *message->Body());
				return XCode::Ok;
			}
		}
		return XCode::Ok;
	}

	int Actor::LuaCall(lua_State* lua, std::unique_ptr<rpc::Packet> message)
	{
		int id = message->SockId();
		this->mLastTime = help::Time::NowSec();
		return this->mRouterComponent->LuaCall(lua, id, std::move(message));
	}

	int Actor::LuaSend(lua_State* lua, std::unique_ptr<rpc::Packet> message)
	{
		int id = message->SockId();
		this->mLastTime = help::Time::NowSec();
		int code = this->mRouterComponent->Send(id, std::move(message));

		lua_pushinteger(lua, code);
		return 1;
	}
}