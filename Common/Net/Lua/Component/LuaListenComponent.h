//
// Created by mac on 2022/5/30.
//

#pragma once;
#include"Lua/Module/LuaModule.h"
#include"Server/Component/ListenerComponent.h"

namespace acs
{
	class LuaListenComponent final : public Component, public ITcpListen
	{
	public:
		LuaListenComponent();
	private:
		bool LateAwake() final;
		bool OnListen(tcp::Socket* socket) final;
	private:
		class Lua::LuaModule* mModule;
	};
}
