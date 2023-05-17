#pragma once
#include"RpcService.h"
namespace Tendo
{
	class LuaRpcService : public RpcService
	{
	private:
		bool OnInit() final { return this->GetLuaModule() != nullptr; }
	};
}// namespace Sentry