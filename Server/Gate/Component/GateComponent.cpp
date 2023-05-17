//
// Created by yjz on 2022/4/23.
//

#include"GateComponent.h"
#include"Entity/Unit/App.h"
#include"Gate/Service/Gate.h"
#include"Gate/Lua/LuaGate.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Lua/Engine/ClassProxyHelper.h"
#include"Rpc/Component/LocationComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Entity/Component/ComponentFactory.h"
namespace Tendo
{
	bool GateComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<LocationComponent>();
        this->mInnerComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}


	int GateComponent::Send(long long userId, const std::string& func)
	{
       
        return XCode::Successful;
	}

	int GateComponent::Send(long long userId, const std::string& func, const Message& message)
	{
      
        return XCode::Successful;
	}

	int GateComponent::BroadCast(const std::string& func)
	{
		
		return XCode::Successful;
	}

	int GateComponent::BroadCast(const std::string& func, const Message& message)
	{
	
        return XCode::Successful;
	}

	void GateComponent::OnLuaRegister(Lua::ClassProxyHelper & luaRegister)
	{
		luaRegister.BeginNewTable("Gate");
		luaRegister.PushExtensionFunction("Send", Lua::Gate::Send);
		luaRegister.PushExtensionFunction("BroadCast", Lua::Gate::BroadCast);
	}

	GateComponent::GateComponent()
	{
		this->mNodeComponent = nullptr;
		this->mInnerComponent = nullptr;
	}
}