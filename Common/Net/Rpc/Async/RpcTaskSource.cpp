#include"RpcTaskSource.h"

#include <utility>
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/TimeHelper.h"
#include"Proto/Include/Message.h"
#include"Proto/Component/ProtoComponent.h"

namespace acs
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua, int id)
		: IRpcTask<rpc::Message>(id), mTask(lua) { }

	void LuaRpcTaskSource::OnResponse(rpc::Message * response)
	{
		int code = XCode::NetTimeout;
		if(response != nullptr)
		{
			response->GetHead().Get("code", code);
		}
		this->mTask.SetResult(code, response);
	}
}
