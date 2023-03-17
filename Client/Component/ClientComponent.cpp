
#include"ClientComponent.h"
#include"App/App.h"
#include"String/StringHelper.h"
#include"Client/TcpRpcClientContext.h"
#include"Lua/Client.h"
#include"Lua/Message.h"
#include"Lua/ClassProxyHelper.h"
#include"google/protobuf/util/json_util.h"
#include"Component/LuaScriptComponent.h"
#include"Component/ThreadComponent.h"
namespace Client
{
	ClientTask::ClientTask(int id)
        : Sentry::IRpcTask<Rpc::Packet>(id)
	{
		this->mTaskId = Helper::Guid::Create();
	}

	void ClientTask::OnResponse(std::shared_ptr<Rpc::Packet> response)
	{
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

    void ClientComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
    {
        int type = message->GetType();
        const std::string& address = message->From();
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
        CONSOLE_LOG_ERROR("unknown message type = " << type);
    }

    void ClientComponent::StartClose(const std::string &address)
    {

    }

    void ClientComponent::OnCloseSocket(const std::string &address, int code)
    {

    }

    bool ClientComponent::LateAwake()
    {
        this->mTimerComponent = this->GetComponent<TimerComponent>();
		return true;
    }

	std::shared_ptr<Rpc::Packet> ClientComponent::Call(int id, std::shared_ptr<Rpc::Packet> request)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return nullptr;
		}
		int rpcId = this->mNumberPool.Pop();
		std::shared_ptr<ClientTask> clientRpcTask(new ClientTask(id));
		{
			request->GetHead().Add("rpc", rpcId);
//			long long timeId = this->mTimerComponent->DelayCall(20 * 1000,
//				[request, rpcId, this]()
//				{
//					std::string func;
//					request->GetHead().Get("func", func);
//					CONSOLE_LOG_ERROR("call " << func << " time out");
//					this->OnResponse(rpcId, nullptr);
//				});
//			this->mTimers.emplace(rpcId, timeId);
		}
		iter->second->SendToServer(request);
		return this->AddTask(rpcId, clientRpcTask)->Await();
	}

	int ClientComponent::New(const std::string& ip, unsigned short port)
	{
		int id = this->mNumberPool.Pop();
        ThreadComponent * netComponent =
			this->GetComponent<ThreadComponent>();

		std::shared_ptr<SocketProxy> socketProxy =
			netComponent->CreateSocket(ip, port);

		std::shared_ptr<TcpRpcClientContext> client =
			std::make_shared<TcpRpcClientContext>(socketProxy, this);
		this->mClients.emplace(id, client);
        return id;
	}

	void ClientComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginNewTable("Client");
        luaRegister.PushExtensionFunction("New", Lua::ClientEx::New);
		luaRegister.PushExtensionFunction("Call", Lua::ClientEx::Call);      
	}
}
