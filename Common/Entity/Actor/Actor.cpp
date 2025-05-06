//
// Created by yjz on 2023/5/17.
//
#include "App.h"
#include "Actor.h"
#include "XCode/XCode.h"
#include "Message/s2s/s2s.pb.h"
#include "Util/Tools/TimeHelper.h"
#include "Proto/Include/Message.h"
#include "Rpc/Config/ServiceConfig.h"
#include "Yyjson/Lua/ljson.h"
#include "Proto/Component/ProtoComponent.h"
#include "Router/Component/RouterComponent.h"
#include "Lua/Lib/Lib.h"
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

	int Actor::Send(const std::string& func) const
	{
		std::unique_ptr<rpc::Message> message = this->Make(func);
		if(message == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		const int id = message->SockId();
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Send(const std::string& func, const pb::Message& request) const
	{
		std::unique_ptr<rpc::Message> message = this->Make(func);
		if(message == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		if(!message->WriteMessage(&request))
		{
			return XCode::SerializationFailure;
		}
		int id = message->SockId();
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Send(std::unique_ptr<rpc::Message> message) const
	{
		int id = message->SockId();
		return this->mRouter->Send(id, std::move(message));
	}

	int Actor::Call(const std::string& func) const
	{
		std::unique_ptr<rpc::Message> message = this->Make(func);
		if(message == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		int id = message->SockId();
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		return result != nullptr ? result->GetCode() : XCode::NetTimeout;
	}
	
	int Actor::Call(const std::string& func, const pb::Message& request) const
	{
		std::unique_ptr<rpc::Message> message = this->Make(func);
		if(message == nullptr)
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

	int Actor::Call(const std::string& func, pb::Message * response) const
	{
		std::unique_ptr<rpc::Message> message = this->Make(func);
		if(message == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		int id = message->SockId();
		std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
		if(result == nullptr)
		{
			return XCode::NetTimeout;
		}
		int code = XCode::Ok;
		if(code == XCode::Ok && !result->ParseMessage(response))
		{
			return XCode::ParseMessageError;
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
		int code = XCode::Ok;
		do
		{
			std::unique_ptr<rpc::Message> message = this->Make(func);
			if(message == nullptr)
			{
				code = XCode::MakeTcpRequestFailure;
				break;
			}
			if (!message->WriteMessage(&request))
			{
				code = XCode::SerializationFailure;
				break;
			}
			int id = message->SockId();
			std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
			if (result == nullptr)
			{
				code = XCode::NetTimeout;
				break;
			}
			code = result->GetCode();
			if (code == XCode::Ok && !result->ParseMessage(response))
			{
				code = XCode::ParseMessageError;
				break;
			}
		} while (false);
		return code;
	}

	int Actor::Send(const std::string& func, const json::w::Document& request)
	{
		int code = XCode::Ok;
		do
		{
			std::unique_ptr<rpc::Message> message = this->Make(func);
			if(message == nullptr)
			{
				code = XCode::MakeTcpRequestFailure;
				break;
			}
			if (!message->WriteMessage(&request))
			{
				code = XCode::SerializationFailure;
				break;
			}
			int id = message->SockId();
			code = this->mRouter->Send(id, std::move(message));
		} while (false);
		return code;
	}

	int Actor::Call(const std::string& func, const json::w::Document& request)
	{
		int code = XCode::Ok;
		do
		{
			std::unique_ptr<rpc::Message> message = this->Make(func);
			if(message == nullptr)
			{
				code = XCode::MakeTcpRequestFailure;
				break;
			}
			if (!message->WriteMessage(&request))
			{
				code = XCode::SerializationFailure;
				break;
			}
			int id = message->SockId();
			std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
			if (result == nullptr)
			{
				code = XCode::NetTimeout;
				break;
			}
			code = result->GetCode();
		}
		while (false);
		return code;
	}

	int Actor::Call(const std::string& func, const json::w::Document& request, json::r::Document* response)
	{
		int code = XCode::Ok;
		do
		{
			std::unique_ptr<rpc::Message> message = this->Make(func);
			if(message == nullptr)
			{
				code = XCode::MakeTcpRequestFailure;
				break;
			}
			if (!message->WriteMessage(&request))
			{
				code = XCode::SerializationFailure;
				break;
			}
			int id = message->SockId();
			std::unique_ptr<rpc::Message> result = this->mRouter->Call(id, std::move(message));
			if (result == nullptr)
			{
				code = XCode::NetTimeout;
				break;
			}
			code = result->GetCode();
			if(code != XCode::Ok)
			{
				break;
			}
			if(!result->ParseMessage(response))
			{
				code = XCode::ParseJsonFailure;
			}
		} while (false);
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
		message = this->Make(func);
		if(message == nullptr)
		{
			return XCode::MakeTcpRequestFailure;
		}
		if(methodConfig->proto == rpc::Proto::Protobuf)
		{
			if (!methodConfig->response.empty())
			{
				message->TempHead().Add("res", methodConfig->response);
			}
		}

		switch (lua_type(lua, idx))
		{
			case LUA_TNIL:
				message->SetProto(rpc::Proto::None);
				return XCode::Ok;
			case LUA_TSTRING:
			{
				size_t count = 0;
				const char* str = lua_tolstring(lua, idx, &count);
				{
					message->Body()->append(str, count);
					message->SetProto(rpc::Proto::String);
				}
				return XCode::Ok;
			}
			case LUA_TNUMBER:
			{
				if(lua_isinteger(lua, idx))
				{
					long long number = luaL_checkinteger(lua, idx);
					message->Body()->append(std::to_string(number));
				}
				else
				{
					double number = luaL_checknumber(lua, idx);
					message->Body()->append(std::to_string(number));
				}
				break;
			}
			case LUA_TTABLE:
			{
				if(methodConfig->proto == rpc::Proto::Protobuf)
				{
					if (!methodConfig->request.empty())
					{
						const std::string& name = methodConfig->request;
						pb::Message* request = this->mProto->Read(lua, name, idx);
						if (request == nullptr)
						{
							LOG_ERROR("call {} fail request : {}", func, name);
							return XCode::CreateProtoFailure;
						}
						message->SetProto(rpc::Proto::Protobuf);
						if (!message->WriteMessage(request))
						{
							return XCode::SerializationFailure;
						}
						return XCode::Ok;
					}
				}
				else if(methodConfig->proto == rpc::Proto::Lua)
				{
					if(lua::lfmt::serialize(lua, idx, *message->Body()) != LUA_OK)
					{
						return XCode::Failure;
					}
					message->SetProto(rpc::Proto::Lua);
					return XCode::Ok;
				}
				message->SetProto(rpc::Proto::Json);
				lua::yyjson::read(lua, idx, *message->Body());
				return XCode::Ok;
			}
		}
		return XCode::Ok;
	}

	int Actor::LuaCall(lua_State* lua, std::unique_ptr<rpc::Message> message) const
	{
		int id = message->SockId();
		return this->mRouter->LuaCall(lua, id, std::move(message));
	}

	int Actor::LuaSend(lua_State* lua, std::unique_ptr<rpc::Message> message) const
	{
		int id = message->SockId();
		int code = this->mRouter->Send(id, std::move(message));

		lua_pushinteger(lua, code);
		return 1;
	}
}