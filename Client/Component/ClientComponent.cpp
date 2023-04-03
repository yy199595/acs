
#include"ClientComponent.h"

#include"Util/String/StringHelper.h"
#include"Client/TcpRpcClientContext.h"
#include"Lua/Client.h"
#include"Proto/Lua/Message.h"
#include"Entity/App/App.h"
#include"Script/Lua/ClassProxyHelper.h"
#include"google/protobuf/util/json_util.h"
#include"Script/Component/LuaScriptComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Proto/Component/ProtoComponent.h"
namespace Client
{
	ClientTask::ClientTask(int id)
        : Sentry::IRpcTask<Rpc::Packet>(id)
	{
		
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
		this->mLuaComponent = nullptr;
		this->mProtoComponent = nullptr;
	}

    void ClientComponent::OnMessage(std::shared_ptr<Rpc::Packet> message)
    {
        int type = message->GetType();
        const std::string &address = message->From();
        switch (type)
        {
            case Tcp::Type::Ping:
            {
                LOG_INFO("[" << address << " ping message = " << message->GetBody());
            }
                break;
            case Tcp::Type::Request:
                this->OnRequest(*message);
                break;
            case Tcp::Type::Response:
            {
                int rpcId = 0;
                if (message->GetHead().Get("rpc", rpcId))
                {
                    this->OnResponse(rpcId, message);
                }
            }
                break;
            default:
            CONSOLE_LOG_ERROR("unknown message type = " << type);
                break;
        }
    }

    void ClientComponent::StartClose(const std::string &address)
    {

    }

    void ClientComponent::OnCloseSocket(const std::string &address, int code)
    {

    }

    bool ClientComponent::LateAwake()
    {
        //this->mTimerComponent = this->GetComponent<TimerComponent>();
        this->mProtoComponent = this->GetComponent<ProtoComponent>();
        this->mLuaComponent = this->GetComponent<LuaScriptComponent>();
		return true;
    }

    bool ClientComponent::Send(int id, const std::shared_ptr<Rpc::Packet>& request, int& rpcId)
    {
        auto iter = this->mClients.find(id);
        if (iter == this->mClients.end())
        {
            return false;
        }

        rpcId = this->mNumberPool.Pop();
        request->GetHead().Add("rpc", rpcId);
        iter->second->SendToServer(request);
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
        std::shared_ptr<ClientTask> clientRpcTask
            = std::make_shared<ClientTask>(rpcId);
		
		iter->second->SendToServer(std::move(request));
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

    void ClientComponent::OnRequest(const Rpc::Packet &message)
	{
		std::string tab, func;
		if (!message.GetMethod(tab, func))
		{
			return;
		}
		if (!this->mLuaComponent->GetFunction(tab, func))
		{
			LOG_ERROR("not find lua function [" << tab << "." << func << "]");
			return;
		}
		int count = 0;
		std::string name;
		if (message.ConstHead().Get("pb", name))
		{
			count++;
			std::shared_ptr<Message> data;
			if (!this->mProtoComponent->New(name, data))
			{
				return;
			}
			if (!data->ParseFromString(message.GetBody()))
			{
				return;
			}
			this->mProtoComponent->Write(this->mLuaComponent->GetLuaEnv(), *data);
		}
		if (lua_pcall(this->mLuaComponent->GetLuaEnv(), count, 0, 0) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLuaComponent->GetLuaEnv(), -1));
		}
	}
}
