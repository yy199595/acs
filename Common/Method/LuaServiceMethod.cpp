#include"LuaServiceMethod.h"
#include"Script/Function.h"
#include"App/App.h"
#include"Pool/MessagePool.h"
#include"Global/ServiceConfig.h"
#include"Async/LuaServiceTaskSource.h"
namespace Sentry
{

	LuaServiceMethod::LuaServiceMethod(const std::string& service, const std::string& func, lua_State* lua)
		: ServiceMethod(func), mLuaEnv(lua)
	{
		this->mFunction = fmt::format("{0}.{1}", service, func);
	}

	XCode LuaServiceMethod::Call(long long id, const std::string& json, com::Rpc::Response & response)
	{
		if(!Lua::Function::Get(this->mLuaEnv, "Service", "Call"))
		{
			return XCode::CallLuaFunctionFail;
		}
		const ServiceConfig & config = App::Get()->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = config.GetInterfaceConfig(this->mFunction);
		if(protoConfig == nullptr || !Lua::Function::Get(this->mLuaEnv,
			protoConfig->Service.c_str(), protoConfig->Method.c_str()))
		{
			return XCode::NotFoundRpcConfig;
		}
		Lua::Parameter::WriteArgs(this->mLuaEnv, id, json);
		if (lua_pcall(this->mLuaEnv, 3, 2, 0) != 0)
		{
			response.set_error_str(lua_tostring(this->mLuaEnv, -1));
			LOG_ERROR("[" << protoConfig->FullName << "] " << response.error_str());
			return XCode::CallLuaFunctionFail;
		}
		const std::string & res = protoConfig->Response;
		XCode code = (XCode)lua_tointeger(this->mLuaEnv, -1);
		if (code != XCode::Successful || res.empty())
		{
			return code;
		}

		size_t size = 0;
		const char* str = lua_tolstring(this->mLuaEnv, -2, &size);
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(res, str, size);
		if(message == nullptr)
		{
			LOG_ERROR("res json = " << std::string(str, size));
			return XCode::JsonCastProtoFailure;
		}
		response.mutable_data()->PackFrom(*message);
		return XCode::Successful;
	}

	XCode LuaServiceMethod::CallAsync(long long id, const std::string& json, com::Rpc::Response & response)
	{
		if(!Lua::Function::Get(this->mLuaEnv, "Service", "CallAsync"))
		{
			return XCode::CallLuaFunctionFail;
		}
		const ServiceConfig & config = App::Get()->GetServiceConfig();
		const RpcInterfaceConfig * protoConfig = config.GetInterfaceConfig(this->mFunction);
		if(protoConfig == nullptr || !Lua::Function::Get(this->mLuaEnv,
			protoConfig->Service.c_str(), protoConfig->Method.c_str()))
		{
			return XCode::NotFoundRpcConfig;
		}
		Lua::Parameter::WriteArgs(this->mLuaEnv, id, json);
		if (lua_pcall(this->mLuaEnv, 3, 1, 0) != 0)
		{
			response.set_error_str(lua_tostring(this->mLuaEnv, -1));
			LOG_ERROR("call lua " << protoConfig->FullName << " " << response.error_str());
			return XCode::CallLuaFunctionFail;
		}
		LuaServiceTaskSource* luaTaskSource = Lua::PtrProxy<LuaServiceTaskSource>::Read(this->mLuaEnv, -1);

		XCode code = luaTaskSource->Await();
		if(code != XCode::Successful)
		{
			response.set_error_str(luaTaskSource->GetJson());
			return code;
		}
		if(!protoConfig->Response.empty())
		{
			std::shared_ptr<Message> message = Helper::Proto::NewByJson(
				protoConfig->Response, luaTaskSource->GetJson());
			if (message == nullptr)
			{
				LOG_ERROR("res json = " << luaTaskSource->GetJson());
				return XCode::JsonCastProtoFailure;
			}
			response.mutable_data()->PackFrom(*message);
		}
		return XCode::Successful;
	}

	XCode LuaServiceMethod::Invoke(const com::Rpc_Request& request, com::Rpc_Response& response)
	{
		const ServiceConfig& config = App::Get()->GetServiceConfig();
		const RpcInterfaceConfig* protoConfig = config.GetInterfaceConfig(this->mFunction);
		if (protoConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		std::string json;
		if (request.has_data() && !Helper::Proto::GetJson(request.data(), json))
		{
			return XCode::ProtoCastJsonFailure;
		}
		return protoConfig->IsAsync ? this->CallAsync(request.user_id(),
			json, response) : this->Call(request.user_id(), json, response);
	}
}
