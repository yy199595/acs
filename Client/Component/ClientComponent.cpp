
#include"ClientComponent.h"

#include"Util/String/StringHelper.h"
#include"Client/TcpRpcClientContext.h"
#include"Lua/Client.h"
#include"Lua/Engine/ClassProxyHelper.h"
#include"google/protobuf/util/json_util.h"
#include"Lua/Component/LuaScriptComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Proto/Component/ProtoComponent.h"
namespace Client
{
	ClientTask::ClientTask(int id)
        : Tendo::IRpcTask<Rpc::Packet>(id)
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
            case Msg::Type::Ping:
            {
                LOG_INFO("[" << address << " ping message = " << message->GetBody());
            }
                break;
            case Msg::Type::Request:
                this->OnRequest(*message);
                break;
            case Msg::Type::Response:
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

	std::shared_ptr<Rpc::Packet> ClientComponent::Call(int id, const std::shared_ptr<Rpc::Packet> & request, bool async)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return nullptr;
		}
		int rpcId = this->mNumberPool.Pop();
		if(async)
		{
			std::shared_ptr<ClientTask> clientRpcTask
					= std::make_shared<ClientTask>(rpcId);
			iter->second->SendToServer(std::move(request));
			return this->AddTask(rpcId, clientRpcTask)->Await();
		}
        iter->second->SendToServer(request, false);
		return iter->second->Receive();
	}

	int ClientComponent::New(const std::string& ip, unsigned short port)
	{
		int id = this->mNumberPool.Pop();
		Asio::Context & io = this->mApp->MainThread();

		std::shared_ptr<Tcp::SocketProxy> socketProxy =
			std::make_shared<Tcp::SocketProxy>(io);

		socketProxy->Init(ip, port);
		std::shared_ptr<TcpRpcClientContext> client =
			std::make_shared<TcpRpcClientContext>(socketProxy, this);
		CONSOLE_LOG_INFO("create new client : [" << socketProxy->GetAddress() << "]");
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

	bool ClientComponent::Close(int id)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return false;
		}
		std::shared_ptr<Rpc::Packet> message
				= std::make_shared<Rpc::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->SetProto(Msg::Porto::Protobuf);
			message->GetHead().Add("func", "Gate.Logout");
			message->GetHead().Add("rpc", this->mNumberPool.Pop());
		}
		std::shared_ptr<TcpRpcClientContext> client = iter->second;
		{
			this->mClients.erase(iter);
			client->SendToServer(message, false);
		}
		client->Receive();
		client->Close();
		return true;
	}

	void ClientComponent::OnDestroy()
	{
		std::vector<int> clients;
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end(); iter++)
		{
			clients.emplace_back(iter->first);
		}
		for(const int & id : clients)
		{
			this->Close(id);
		}
	}
}
