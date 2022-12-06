
#include"ClientComponent.h"
#include"App/App.h"
#include"String/StringHelper.h"
#include"Client/TcpRpcClientContext.h"
#include"Lua/Client.h"
#include"Lua/Message.h"
#include"google/protobuf/util/json_util.h"
#include"Component/LuaScriptComponent.h"
#include"Component/NetThreadComponent.h"
namespace Client
{
	ClientTask::ClientTask(int ms)
        : Sentry::IRpcTask<Rpc::Packet>(ms)
	{
		this->mTaskId = Guid::Create();
	}

	void ClientTask::OnResponse(std::shared_ptr<Rpc::Packet> response)
	{
		this->mTask.SetResult(response);
	}

    void ClientTask::OnTimeout()
    {
        std::shared_ptr<Rpc::Packet> response(new Rpc::Packet());

        response->GetHead().Add("code", (int)XCode::CallTimeout);
        this->mTask.SetResult(response);
    }

}

namespace Client
{
	ClientComponent::ClientComponent()
	{
        this->mIndex = 0;
        this->mTimerComponent = nullptr;
	}

    void ClientComponent::OnRequest(std::shared_ptr<c2s::rpc::call> t1)
    {
        LOG_INFO("call client func = " << t1->func());
	}

    void ClientComponent::OnMessage(const std::string &address, std::shared_ptr<Rpc::Packet> message)
    {
        int type = message->GetType();       
        switch (type)
        {
            case (int)Tcp::Type::Ping:
            {
                LOG_INFO("[" << address << " ping message = " << message->GetBody());
            }
                return;
            case (int)Tcp::Type::Request:
            {
                std::string func;
                if(message->GetHead().Get("func", func))
                {
                    LOG_ERROR("server request client func : [" << func << "]");
                }
            }
                return;
            case (int)Tcp::Type::Broadcast:
            {
                std::string func;
                if(message->GetHead().Get("func", func))
                {
                    LOG_ERROR("server broadcast client func : [" << func << "]");
                }
            }
                return;
            case (int)Tcp::Type::Response:
            {
                long long rpcId = 0;
                if (message->GetHead().Get("rpc", rpcId))
                {
                    this->OnResponse(rpcId, message);
                }
            }
                return;
        }
        CONSOLE_LOG_ERROR("unknow message type = " << type);
    }

    void ClientComponent::StartClose(const std::string &address)
    {

    }

    void ClientComponent::OnCloseSocket(const std::string &address, XCode code)
    {

    }

    bool ClientComponent::LateAwake()
    {
        this->mTimerComponent = this->GetComponent<TimerComponent>();
		return true;
    }

	std::shared_ptr<Rpc::Packet> ClientComponent::Call(std::shared_ptr<Rpc::Packet> request)
	{
		std::shared_ptr<ClientTask> clienRpcTask(new ClientTask(0));
		{
			long long taskId = clienRpcTask->GetRpcId();
			request->GetHead().Add("rpc", taskId);
			long long timeId = this->mTimerComponent->DelayCall(20 * 1000,
				[request, taskId, this]()
				{
					std::string func;
					request->GetHead().Get("func", func);
					CONSOLE_LOG_ERROR("call " << func << " time out");
					this->OnResponse(taskId, nullptr);
				});
			this->mTimers.emplace(taskId, timeId);
		}
		this->mTcpClient->SendToServer(request);
		return this->AddTask(clienRpcTask->GetRpcId(), clienRpcTask)->Await();
	}

	void ClientComponent::OnAddTask(RpcTask rpctask)
	{
		//LOG_WARN(this->GetName() << " add new task " << rpctask->GetRpcId());
	}

	void ClientComponent::OnDelTask(long long key, RpcTask task)
	{
		auto iter = this->mTimers.find(key);
		if(iter != this->mTimers.end())
		{
            long long timerId = iter->second;
            this->mTimerComponent->CancelTimer(timerId);
			this->mTimers.erase(iter);
		}
	}

	bool ClientComponent::New(const std::string& ip, unsigned short port)
	{
        if (this->mTcpClient != nullptr)
        {
            return false;
        }
        NetThreadComponent * netComponent = this->GetComponent<NetThreadComponent>();
		std::shared_ptr<SocketProxy> socketProxy = netComponent->CreateSocket(ip, port);
		this->mTcpClient = std::make_shared<TcpRpcClientContext>(socketProxy, this);
        return true;
	}

	void ClientComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginNewTable("Client");
        luaRegister.PushExtensionFunction("New", Lua::ClientEx::New);
		luaRegister.PushExtensionFunction("Call", Lua::ClientEx::Call);      
	}
}
