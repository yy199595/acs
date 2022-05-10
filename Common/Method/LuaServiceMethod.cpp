#include"LuaServiceMethod.h"
#include"Script/Function.h"
#include"App/App.h"
#include"Pool/MessagePool.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Global/ServiceConfig.h"
#include"Async/LuaServiceTaskSource.h"
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const std::string& service, const std::string& func, lua_State* lua)
		: ServiceMethod(func), mLuaEnv(lua)
	{
		this->mFunction = fmt::format("{0}.{1}", service, func);
	}

	tuple<XCode, std::shared_ptr<Message>> LuaServiceMethod::Call(long long id, const std::string& json)
	{
		if(!Lua::Function::Get(this->mLuaEnv, "Service", "Call"))
		{
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		const ServiceConfig & config = App::Get()->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = config.GetInterfaceConfig(this->mFunction);
		if(protoConfig == nullptr)
		{
			return make_tuple(XCode::NotFoundRpcConfig, nullptr);
		}
		const char * tab = protoConfig->Service.c_str();
		const char * func = protoConfig->Method.c_str();
		if(!Lua::Function::Get(this->mLuaEnv, tab, func))
		{
			return make_tuple(XCode::NotFoundRpcConfig, nullptr);
		}
		Lua::Parameter::WriteArgs(this->mLuaEnv, id, json);
		if (lua_pcall(this->mLuaEnv, 3, 2, 0) != 0)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			return make_tuple(XCode::CallLuaFunctionFail, nullptr);
		}
		const std::string & response = protoConfig->Response;
		XCode code = (XCode)lua_tointeger(this->mLuaEnv, -1);
		if (code != XCode::Successful || response.empty())
		{
			return make_tuple(code, nullptr);
		}

		size_t size = 0;
		const char* str = lua_tolstring(this->mLuaEnv, -2, &size);
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(response, str, size);
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
		const ServiceConfig & config = App::Get()->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = config.GetInterfaceConfig(this->mFunction);
		if(protoConfig == nullptr)
		{
			return make_tuple(XCode::NotFoundRpcConfig, nullptr);
		}
		const std::string& name = protoConfig->Response;
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
		const ServiceConfig & config = App::Get()->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = config.GetInterfaceConfig(this->mFunction);
		if(protoConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::string json;
		if (request.has_data() && !Helper::Proto::GetJson(request.data(), json))
		{
			return XCode::ProtoCastJsonFailure;
		}
		auto luaResponse = protoConfig->IsAsync
						   ? this->CallAsync(request.user_id(), json) : this->Call(request.user_id(), json);
		if (std::get<1>(luaResponse) != nullptr)
		{
			auto message = std::get<1>(luaResponse);
			response.mutable_data()->PackFrom(*message);
		}
		return std::get<0>(luaResponse);
	}
}
