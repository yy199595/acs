#include"RpcTaskSource.h"
#include"Time/TimeHelper.h"
#include"Component/ProtoComponent.h"
namespace Sentry
{
    RpcTaskSource::RpcTaskSource(int ms)
        : IRpcTask<Rpc::Packet>(ms)
    {

    }
    void RpcTaskSource::OnResponse(std::shared_ptr<Rpc::Packet> response)
    {
        this->mTaskSource.SetResult(response);
    }

    void RpcTaskSource::OnTimeout()
    {
        this->mTaskSource.SetResult(nullptr);
    }

	std::shared_ptr<Rpc::Packet> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}

namespace Sentry
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua, int ms, const std::string & resp)
		: IRpcTask<Rpc::Packet>(ms), mTask(lua), mResp(resp)
	{
		this->mTaskId = Helper::Guid::Create();
#ifdef __DEBUG__
      this->t1 = Helper::Time::NowMilTime();
#endif
	}

    void LuaRpcTaskSource::OnTimeout()
    {
        this->mTask.SetResult(XCode::CallTimeout, nullptr);
    }

	void LuaRpcTaskSource::OnResponse(std::shared_ptr<Rpc::Packet> response)
	{
#ifdef __DEBUG__
        std::string func;
        long long t2 = Helper::Time::NowMilTime();
        if(response->GetHead().Get("func", func))
        {
            long long ms = t2 - this->t1;
            CONSOLE_LOG_INFO("lua call " << func << " use time [" << ms << "ms]");
        }
#endif
        int code = 0;
        response->GetHead().Get("code", code);
        if(code == (int)XCode::Successful && !this->mResp.empty())
        {
            ProtoComponent * messageComponent = App::Inst()->GetMsgComponent();
            std::shared_ptr<Message> message = messageComponent->New(this->mResp);
            if(message != nullptr && response->ParseMessage(message.get()))
            {
                this->mTask.SetResult(XCode::Successful, message);
            }
            return;
        }
		this->mTask.SetResult(code, nullptr);
	}
}
