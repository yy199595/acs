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
		this->mIndex = 0;
		this->mProto = nullptr;
		this->mDisComponent = nullptr;
		this->mLuaComponent = nullptr;
	}

	bool ClientComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mProto = this->GetComponent<ProtoComponent>())
		LOG_CHECK_RET_FALSE(this->mLuaComponent = this->GetComponent<LuaComponent>())
		LOG_CHECK_RET_FALSE(this->mDisComponent = this->GetComponent<DispatchComponent>())
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
		int id = ++this->mIndex;
		Asio::Context & context = this->mApp->GetContext();
		std::unique_ptr<tcp::Socket> tcpSocket = std::make_unique<tcp::Socket>(context);
		std::shared_ptr<rpc::InnerClient> client = std::make_shared<rpc::InnerClient>(id, this, true, context);
		{
			tcpSocket->Init(ip, port);
			client->SetSocket(tcpSocket.release());
			this->mClientMap.emplace(id, std::move(client));
		}
		return id;
	}

	int ClientComponent::Send(int id, rpc::Message * message)
	{
		auto iter = this->mClientMap.find(id);
		if(iter == this->mClientMap.end())
		{
			return XCode::NetWorkError;
		}
		return iter->second->Send(message) ? XCode::Ok : XCode::SendMessageFail;
	}

	void ClientComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{
		luaRegister.AddFunction("Send", LuaClient::Send);
		luaRegister.AddFunction("Call", LuaClient::Call);
		luaRegister.AddFunction("Close", LuaClient::Close);
		luaRegister.AddFunction("Connect", LuaClient::Connect);
		luaRegister.End("net.client");
	}

	void ClientComponent::OnSendFailure(int id, rpc::Message* message)
	{
		if(message->GetType() == rpc::Type::Request && message->GetRpcId() > 0)
		{
			message->SetType(rpc::Type::Response);
			message->GetHead().Add(rpc::Header::code, XCode::SendMessageFail);
			this->OnMessage(message, nullptr);
			return;
		}
		delete message;
	}

	void ClientComponent::OnMessage(rpc::Message* message, rpc::Message* response)
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

	int ClientComponent::Remove(int id)
	{
		auto iter = this->mClientMap.find(id);
		if(iter == this->mClientMap.end())
		{
			return 0;
		}
		iter->second->Close();
		this->mClientMap.erase(iter);
		return 1;
	}

	void ClientComponent::OnClientError(int id, int code)
	{
		auto iter = this->mClientMap.find(id);
		if(iter == this->mClientMap.end())
		{
			return;
		}
		this->mClientMap.erase(iter);
	}

	int ClientComponent::OnRequest(rpc::Message* message)
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
		if(lua_pcall(lua, count, 1, 0) != LUA_OK)
		{
			LOG_ERROR("{}", lua_tostring(lua, -1))
			lua_pop(lua, 1);
		}
		return XCode::Ok;
	}
}