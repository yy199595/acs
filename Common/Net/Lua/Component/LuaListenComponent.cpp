
#include"LuaListenComponent.h"
#include"Lua/Component/LuaComponent.h"
namespace acs
{
	LuaListenComponent::LuaListenComponent()
	{
		this->mModule = nullptr;
	}

	bool LuaListenComponent::LateAwake()
	{
		LuaComponent* luaComponent = this->GetComponent<LuaComponent>();
		LOG_CHECK_RET_FALSE(this->mModule = luaComponent->LoadModule("LuaListenComponent"))
		return true;
	}

	bool LuaListenComponent::OnListen(tcp::Socket* socket)
	{
		return this->mModule->Call("OnListen", socket) == XCode::Ok;
	}
}