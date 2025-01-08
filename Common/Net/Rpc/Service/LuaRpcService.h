#pragma once
#include"RpcService.h"
namespace acs
{
	class LuaRpcService final : public RpcService, public ILogin
	{
	private:
		void OnLogin(long long playerId) noexcept final;
		void OnLogout(long long playerId) noexcept final;
		bool OnInit() final { return this->GetLuaModule() != nullptr; }
	};
}// namespace Sentry