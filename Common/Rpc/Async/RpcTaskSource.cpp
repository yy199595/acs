#include"RpcTaskSource.h"
#include"Time/TimeHelper.h"
#include"Component/ProtoComponent.h"
namespace Sentry
{
    RpcTaskSource::RpcTaskSource(int ms)
        : IRpcTask<Rpc::Data>(ms)
    {
#ifdef __DEBUG__
        this->t1 = Helper::Time::GetNowMilTime();
#endif
    }
    void RpcTaskSource::OnResponse(std::shared_ptr<Rpc::Data> response)
    {
#ifdef __DEBUG__
        std::string func;
        long long t2 = Helper::Time::GetNowMilTime();
        if(response->GetHead().Get("func", func))
        {
            long long ms = t2 - this->t1;
            CONSOLE_LOG_INFO("call " << func << " use time [" << ms << "ms]");
        }
#endif
        this->mTaskSource.SetResult(response);
    }

    void RpcTaskSource::OnTimeout()
    {
        this->mTaskSource.SetResult(nullptr);
    }

	std::shared_ptr<Rpc::Data> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}

namespace Sentry
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua, int ms, const std::string & resp)
		: IRpcTask<Rpc::Data>(ms), mTask(lua), mResp(resp)
	{
		this->mTaskId = Guid::Create();
#ifdef __DEBUG__
      this->t1 = Helper::Time::GetNowMilTime();
#endif
	}

    void LuaRpcTaskSource::OnTimeout()
    {
        this->mTask.SetResult(XCode::CallTimeout, nullptr);
    }

	void LuaRpcTaskSource::OnResponse(std::shared_ptr<Rpc::Data> response)
	{
#ifdef __DEBUG__
        std::string func;
        long long t2 = Helper::Time::GetNowMilTime();
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
            ProtoComponent * messageComponent = App::Get()->GetMsgComponent();
            std::shared_ptr<Message> message = messageComponent->New(this->mResp);
            if(message != nullptr && response->ParseMessage(message))
            {
                this->mTask.SetResult(XCode::Successful, message);
            }
            return;
        }
		this->mTask.SetResult((XCode)code, nullptr);
	}
}
