#include"LuaServiceMgrComponent.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Service/LuaRpcService.h"
#include"Component/Lua/LuaScriptComponent.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Util/DirectoryHelper.h"
namespace Sentry
{

	bool LuaServiceMgrComponent::Awake()
	{

		return true;
	}

	void LuaServiceMgrComponent::OnHotFix()
	{

	}

	bool LuaServiceMgrComponent::LateAwake()
	{
		LOGIC_THROW_ERROR(this->mLuaComponent = this->GetComponent<LuaScriptComponent>());
		LOGIC_THROW_ERROR(this->mConfigComponent = this->GetComponent<RpcConfigComponent>());

		std::vector<std::string> services;
		const ServerConfig& config = App::Get()->GetConfig();
		LOGIC_THROW_ERROR(config.GetMember("service", services));

		for (std::string& service : services)
		{
			auto luaService = this->GetComponent<RpcServiceBase>(service);
			if (luaService != nullptr && !this->AddMethod(luaService))
			{
				LOG_ERROR(luaService->GetName(), " add lua method failure");
				return false;
			}
		}
		return true;
	}

	bool LuaServiceMgrComponent::AddMethod(RpcServiceBase* rpcService)
	{
		std::vector<std::string> methods;
		const std::string& name = rpcService->GetName();
		if (!this->mConfigComponent->GetMethods(name, methods))
		{
			return true;
		}
		lua_State* lua = this->mLuaComponent->GetLuaEnv();
		for (std::string& method : methods)
		{
			if (!Lua::lua_getfunction(lua, name.c_str(), method.c_str()))
			{
				continue;
			}
			int idx = luaL_ref(lua, LUA_REGISTRYINDEX);
			auto config = this->mConfigComponent
				->GetProtocolConfig(name + "." + method);
			if (!rpcService->AddMethod(std::make_shared<LuaServiceMethod>(config, lua, idx)))
			{
				return false;
			}
		}
		return true;
	}
}
