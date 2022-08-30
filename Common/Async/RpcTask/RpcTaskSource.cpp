#include"RpcTaskSource.h"
#include"Component/Scene/ProtocolComponent.h"
namespace Sentry
{
    void RpcTaskSource::OnResponse(std::shared_ptr<com::rpc::response> response)
    {
        this->mTaskSource.SetResult(response);
    }

    void RpcTaskSource::OnTimeout()
    {
        std::shared_ptr<com::rpc::response> response(new com::rpc::response());

        response->set_error_str("rpc call time out");
        response->set_code((int)XCode::CallTimeout);
        this->mTaskSource.SetResult(response);
    }

	std::shared_ptr<com::rpc::response> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}

namespace Sentry
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua, int ms)
		: IRpcTask<com::rpc::response>(ms), mTask(lua)
	{
		this->mTaskId = Guid::Create();
	}

    void LuaRpcTaskSource::OnTimeout()
    {
        this->mTask.SetResult(XCode::CallTimeout, nullptr);
    }

	void LuaRpcTaskSource::OnResponse(std::shared_ptr<com::rpc::response> response)
	{
		XCode code = (XCode)response->code();
		if(code == XCode::Successful && response->has_data())
		{
			ProtocolComponent * messageComponent = App::Get()->GetMsgComponent();
			std::shared_ptr<Message> message = messageComponent->New(response->data());
			this->mTask.SetResult(XCode::Successful, message);
			return;
		}
		this->mTask.SetResult(code, nullptr);
	}
}
