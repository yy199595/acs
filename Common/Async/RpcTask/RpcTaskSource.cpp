#include "RpcTaskSource.h"
#include"Object/App.h"
#include"Rpc/RpcComponent.h"
#ifdef __DEBUG__
#include"Scene/RpcConfigComponent.h"
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

    XCode RpcTaskSource::GetCode()
    {
        auto response = this->mTaskSource.Await();
        if (response == nullptr) {
            return XCode::CallTimeout;
        }
        return (XCode) response->code();
    }
}
