#include"RpcService.h"
#include"Rpc/Lua/LuaServiceMethod.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Lua/Component/LuaScriptComponent.h"
namespace Tendo
{
	bool RpcService::LateAwake()
	{
		const std::string & name = this->GetName();
		LuaScriptComponent * luaComponent = this->GetComponent<LuaScriptComponent>();
		LOG_CHECK_RET_FALSE(ClusterConfig::Inst()->GetServerName(name, this->mCluster));
		if(luaComponent != nullptr)
		{
			this->mLuaModule = luaComponent->LoadModule(name);
		}
		return this->OnInit();
	}
}
