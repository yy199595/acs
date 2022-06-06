#include"RpcTaskSource.h"
#include"Component/Scene/MessageComponent.h"
#include"Component/Rpc/RpcHandlerComponent.h"
namespace Sentry
{
    void RpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        this->mTaskSource.SetResult(response);
    }

	std::shared_ptr<com::Rpc_Response> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}

namespace Sentry
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua)
		: mTask(lua)
	{
		this->mTaskId = Guid::Create();
	}

	void LuaRpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
		if(response == nullptr)
		{
			this->mTask.SetResult(XCode::CallTimeout, nullptr);
			return;
		}
		XCode code = (XCode)response->code();
		if(code == XCode::Successful && response->has_data())
		{
			MessageComponent * messageComponent = App::Get()->GetMsgComponent();
			std::shared_ptr<Message> message = messageComponent->New(response->data());
			this->mTask.SetResult(XCode::Successful, message);
			return;
		}
		this->mTask.SetResult(code, nullptr);
	}
}
