//
// Created by leyi on 2023/9/11.
//

#include "ClientComponent.h"
#include "Yyjson/Lua/ljson.h"
#include "Util/File/FileHelper.h"
#include "Util/Tools/String.h"
#include "Lua/Component/LuaComponent.h"
#include "Client/Lua/LuaClient.h"
#include"Lua/Engine/ModuleClass.h"
#include "Proto/Component/ProtoComponent.h"
#include "Rpc/Component/DispatchComponent.h"
namespace acs
{
	ClientComponent::ClientComponent() : ISender(rpc::Net::Client)
	{
		this->mProto = nullptr;
		this->mDisComponent = nullptr;
		this->mLuaComponent = nullptr;
	}

	bool ClientComponent::LateAwake()
	{
		this->mProto = this->GetComponent<ProtoComponent>();
		this->mLuaComponent = this->GetComponent<LuaComponent>();
		this->mDisComponent = this->GetComponent<DispatchComponent>();
		return true;
	}

	int ClientComponent::Connect(const std::string& address)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return -1;
		}
		static int index = 1;
		Asio::Context & context = App::GetContext();
		tcp::Socket * tcpSocket = new tcp::Socket(context);
		{
			tcpSocket->Init(ip, port);
		}
		int id = index++;
		rpc::InnerClient * client = new rpc::InnerClient(id, this);
		{
			client->SetSocket(tcpSocket);
			this->mClientMap.Add(id, client);
		}
		return id;
	}

	int ClientComponent::Send(int id, rpc::Packet * message)
	{
		rpc::InnerClient * client = nullptr;
		if(!this->mClientMap.Get(id, client))
		{
			return XCode::NetWorkError;
		}
		client->Send(message);
		return XCode::Ok;
	}

	void ClientComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{
		luaRegister.AddFunction("Send", LuaClient::Send);
		luaRegister.AddFunction("Call", LuaClient::Call);
		luaRegister.AddFunction("Connect", LuaClient::Connect);
		luaRegister.End("net.client");
	}

	void ClientComponent::OnMessage(rpc::Packet* message, rpc::Packet* response)
	{
		int code = XCode::Failure;
		switch(message->GetType())
		{
			case rpc::Type::Request:
				this->OnRequest(message);
				break;
			case rpc::Type::Response:
				code = this->mDisComponent->OnMessage(message);
				break;
		}
		if(code != XCode::Ok)
		{
			delete message;
		}
	}

	int ClientComponent::OnRequest(rpc::Packet* message)
	{
		const std::string & func = message->GetHead().GetStr("func");
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		Lua::LuaModule * luaModule = this->mLuaComponent->LoadModule(methodConfig->Service);
		if(luaModule == nullptr)
		{
			LOG_ERROR("not find lua client module : {}", methodConfig->Service);
			return XCode::CallServiceNotFound;
		}
		lua_State * lua = luaModule->GetLuaEnv();
		luaModule->GetFunction(methodConfig->Method);
		const std::string & body = message->GetBody();

		int count = 1;
		switch(message->GetProto())
		{
			case rpc::Porto::String:
				count++;
				lua_pushlstring(lua, body.c_str(), body.size());
				break;
			case rpc::Porto::Json:
				count++;
				lua::yyjson::write(lua, body.c_str(), body.size());
				break;
			case rpc::Porto::Protobuf:
			{
				pb::Message * request = this->mProto->Temp(methodConfig->Request);
				if(request != nullptr)
				{
					count++;
					message->ParseMessage(request);
					this->mProto->Write(lua, *request);
				}
				break;
			}
		}
		lua_pcall(lua, count, 1, 0);
		return XCode::Ok;
	}
}