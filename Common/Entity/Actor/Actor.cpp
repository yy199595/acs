//
// Created by yjz on 2023/5/17.
//
#include"App.h"
#include"Actor.h"
#include"XCode/XCode.h"
#include"Util/Tools/TimeHelper.h"
#include"Proto/Include/Message.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Yyjson/Lua/ljson.h"
#include"Proto/Component/ProtoComponent.h"
#include"Router/Component/RouterComponent.h"
#include "Message/s2s/s2s.pb.h"
namespace acs
{
	Actor::Actor(long long id, std::string  name)
		: Entity(id), mName(std::move(name))
	{
		this->mProto = nullptr;
		this->mRouter = nullptr;
	}

	bool Actor::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mProto = App::Get<ProtoComponent>())
		LOG_CHECK_RET_FALSE(this->mRouter = App::Get<RouterComponent>())
		return this->OnInit();
	}

	int Actor::Send(const std::string& func)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Send(const std::string& func, const pb::Message& request)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		if(!message->WriteMessage(&request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Call(const std::string& func)
	{
		std::unique_ptr<rpc::Message> message ;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		return result != nullptr ? result->GetCode() : XCode::NetTimeout;
	}
	
	int Actor::Call(const std::string& func, const pb::Message& request)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return XCode::MakeTcpRequestFailure;
		}
		if(!message->WriteMessage(&request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		return result != nullptr ? result->GetCode() : XCode::NetWorkError;
	}

	int Actor::Call(const std::string& func, pb::Message * response)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make(func, message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		if(result == nullptr)
		{
			return XCode::NetTimeout;
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

	int Actor::Call(std::unique_ptr<rpc::Message> message)
	{
		int id = 0;
		if(!this->GetAddress(*message, id))
		{
			return XCode::NotFoundActorAddress;
		}
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		if(result == nullptr)
		{
			return XCode::NetTimeout;
		}
		return result->GetCode();
	}

	int Actor::Call(const std::string& func, const pb::Message& request, pb::Message * response)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make(func, message);
		if (code != XCode::Ok)
		{
			return code;
		}
		if(!message->WriteMessage(&request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		if (result == nullptr)
		{
			return XCode::NetTimeout;
		}
		code = result->GetCode();
		if (code == XCode::Ok && !result->ParseMessage(response))
		{
			return XCode::ParseMessageError;
		}
		return code;
	}

	int Actor::MakeMessage(lua_State* lua, int idx,
		const std::string& func, std::unique_ptr<rpc::Message> & message) const
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
		if (!methodConfig->Response.empty())
		{
			message->TempHead().Add("res", methodConfig->Response);
		}
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
				}
				return XCode::Ok;
			}
			case LUA_TTABLE:
			{
				if (!methodConfig->Request.empty())
				{
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
					return XCode::Ok;
				}
				message->SetProto(rpc::Porto::Json);
				lua::yyjson::read(lua, idx, *message->Body());
				return XCode::Ok;
			}
		}
		return XCode::Ok;
	}

	int Actor::LuaCall(lua_State* lua, std::unique_ptr<rpc::Message> message)
	{
		int id = message->SockId();
		return this->mRouter->LuaCall(lua, id, std::move(message));
	}

	int Actor::LuaSend(lua_State* lua, std::unique_ptr<rpc::Message> message)
	{
		int id = message->SockId();
		int code = this->mRouter->Send(id, std::move(message));

		lua_pushinteger(lua, code);
		return 1;
	}

	int Actor::Publish(const std::string& event)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make("EventSystem.Publish", message);
		if(code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		message->SetProto(rpc::Porto::String);
		message->GetHead().Add("channel", event);
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Publish(const std::string& event, json::w::Document& document)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make("EventSystem.Publish", message);
		if (code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		message->SetProto(rpc::Porto::Json);
		message->GetHead().Add("channel", event);
		message->SetContent(document.JsonString());
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Publish(const std::string& event, char proto, const std::string& data)
	{
		std::unique_ptr<rpc::Message> message;
		int code = this->Make("EventSystem.Publish", message);
		if (code != XCode::Ok)
		{
			return code;
		}
		int id = message->SockId();
		message->SetContent(proto, data);
		message->SetProto(rpc::Porto::Json);
		message->GetHead().Add("channel", event);
		return this->mRouter->Send(id, std::move(message));
	}
}