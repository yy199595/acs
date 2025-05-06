#include"RpcTaskSource.h"
#include"XCode/XCode.h"
#include"Util/Tools/TimeHelper.h"
#include"Proto/Component/ProtoComponent.h"

namespace acs
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua, int id)
		: IRpcTask<rpc::Message>(id), mTask(lua) { }

	void LuaRpcTaskSource::OnResponse(std::unique_ptr<rpc::Message> response) noexcept
	{
		int code = response == nullptr
				? XCode::NetTimeout : response->GetCode();
		this->mTask.SetResult(code, std::move(response));
	}
}
