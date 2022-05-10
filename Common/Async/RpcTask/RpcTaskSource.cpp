#include "RpcTaskSource.h"
#include"App/App.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#ifdef __DEBUG__
#include"Global/ServiceConfig.h"
#endif
namespace Sentry
{
    LuaRpcTaskSource::LuaRpcTaskSource(lua_State *lua, lua_State *coroutine)
    {
        this->mluaEnv = lua;
        this->mCoroutine = coroutine;
        this->mRpcId = Helper::Guid::Create();
    }

    void LuaRpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {

    }
}


namespace Sentry
{
    void RpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        this->mTaskSource.SetResult(response);
    }

    XCode RpcTaskSource::AwaitCode()
    {
        auto response = this->mTaskSource.Await();
        if (response == nullptr) {
            return XCode::CallTimeout;
        }
        return (XCode) response->code();
    }

	std::shared_ptr<com::Rpc_Response> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}
