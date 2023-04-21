#include"RpcTaskSource.h"
#include"XCode/XCode.h"
#include"Entity/Unit/App.h"
#include"Util/Time/TimeHelper.h"
#include"Proto/Component/ProtoComponent.h"
namespace Tendo
{
    RpcTaskSource::RpcTaskSource(int ms)
        : IRpcTask<Msg::Packet>(ms)
    {

    }
    void RpcTaskSource::OnResponse(std::shared_ptr<Msg::Packet> response)
    {
        this->mTaskSource.SetResult(response);
    }

	std::shared_ptr<Msg::Packet> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}

namespace Tendo
{
	LuaRpcTaskSource::LuaRpcTaskSource(lua_State* lua, int id, const std::string & resp)
		: IRpcTask<Msg::Packet>(id), mTask(lua), mResp(resp)
	{
#ifdef __DEBUG__
      this->t1 = Helper::Time::NowMilTime();
#endif
	}

	void LuaRpcTaskSource::OnResponse(std::shared_ptr<Msg::Packet> response)
	{
#ifdef __RPC_MESSAGE__
		std::string func;
		long long t2 = Helper::Time::NowMilTime();
		if (response->GetHead().Get("func", func))
		{
			long long ms = t2 - this->t1;
			CONSOLE_LOG_INFO("lua call " << func << " use time [" << ms << "ms]");
		}
#endif
		int code = 0;
		std::shared_ptr<Message> message;
		response->GetHead().Get("code", code);
		if (code == (int)XCode::Successful && !this->mResp.empty())
		{
			ProtoComponent* messageComponent = App::Inst()->GetProto();
			if(!messageComponent->New(this->mResp, message))
			{
				code = XCode::CreateProtoFailure;
			}
			else if(!response->ParseMessage(message.get()))
			{
				code = XCode::ParseMessageError;
			}
		}
		this->mTask.SetResult(code, message);
	}
}
