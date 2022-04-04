#include"LuaServiceMethod.h"
#include"Script/Function.h"
#include"App/App.h"
#include"Pool/MessagePool.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Async/LuaServiceTaskSource.h"
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const ProtoConfig* config, lua_State* lua, int idx)
		: ServiceMethod(config->Method), mLuaEnv(lua), mIdx(idx)
	{
		this->mProtoConfig = config;
	}

	tuple<XCode, std::shared_ptr<Message>> LuaServiceMethod::Call(long long id, const std::string& json)
	{
		if(!Lua::Function::Get(this->mLuaEnv, "Service", "Call"))
		{
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		Lua::Parameter::WriteArgs(this->mLuaEnv, id, json);
		if (lua_pcall(this->mLuaEnv, 3, 2, 0) != 0)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		XCode code = (XCode)lua_tointeger(this->mLuaEnv, -1);
		const std::string& name = this->mProtoConfig->Response;
		if (code != XCode::Successful || name.empty())
		{
			return make_tuple(code, nullptr);
		}

		size_t size = 0;
		const char* str = lua_tolstring(this->mLuaEnv, -2, &size);
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(name, str, size);
		return make_tuple(XCode::Successful, message);
	}

	tuple<XCode, std::shared_ptr<Message>> LuaServiceMethod::CallAsync(long long id, const std::string& json)
	{
		if(!Lua::Function::Get(this->mLuaEnv, "Service", "Call"))
		{
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		lua_pushinteger(this->mLuaEnv, id);
		lua_pushlstring(this->mLuaEnv, json.c_str(), json.size());
		if (lua_pcall(this->mLuaEnv, 3, 1, 0) != 0)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		LuaServiceTaskSource* luaTaskSource = Lua::PtrProxy<LuaServiceTaskSource>::Read(this->mLuaEnv, -1);
		if (luaTaskSource == nullptr)
		{
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		XCode code = luaTaskSource->Await();
		const std::string& name = this->mProtoConfig->Response;
		if (code != XCode::Successful || name.empty())
		{
			return make_tuple(code, nullptr);
		}
		const std::string& response = luaTaskSource->GetJson();
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(name, response);
		return make_tuple(XCode::Successful, message);
	}

	XCode LuaServiceMethod::Invoke(const com::Rpc_Request& request, com::Rpc_Response& response)
	{
		std::string json;
		if (request.has_data() && !Helper::Proto::GetJson(request.data(), json))
		{
			return XCode::ProtoCastJsonFailure;
		}
		auto luaResponse = this->mProtoConfig->IsAsync
						   ? this->CallAsync(request.user_id(), json) : this->Call(request.user_id(), json);
		if (std::get<1>(luaResponse) != nullptr)
		{
			auto message = std::get<1>(luaResponse);
			response.mutable_data()->PackFrom(*message);
		}
		return std::get<0>(luaResponse);
	}
}
