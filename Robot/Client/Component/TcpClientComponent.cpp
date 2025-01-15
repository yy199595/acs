//
// Created by leyi on 2023/9/11.
//

#include "TcpClientComponent.h"
#include "Yyjson/Lua/ljson.h"
#include "Util/Tools/String.h"
#include "Lua/Component/LuaComponent.h"
#include "Client/Lua/LuaClient.h"
#include"Lua/Engine/ModuleClass.h"
#include "Proto/Component/ProtoComponent.h"
#include "Rpc/Component/DispatchComponent.h"
namespace acs
{
	TcpClientComponent::TcpClientComponent()
	{
		this->mIndex = 0;
		this->mProto = nullptr;
		this->mDisComponent = nullptr;
		this->mLuaComponent = nullptr;
	}

	bool TcpClientComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mProto = this->GetComponent<ProtoComponent>())
		LOG_CHECK_RET_FALSE(this->mLuaComponent = this->GetComponent<LuaComponent>())
		LOG_CHECK_RET_FALSE(this->mDisComponent = this->GetComponent<DispatchComponent>())
		this->mLuaComponent->AddCCModule("net.client", lua::lib::luaopen_lclient);
		return true;
	}

	int TcpClientComponent::Connect(const std::string& address)
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
		std::shared_ptr<rpc::InnerTcpClient> client = std::make_shared<rpc::InnerTcpClient>(id, this, true, context);
		{
			tcpSocket->Init(ip, port);
			client->SetSocket(tcpSocket.release());
			this->mClientMap.emplace(id, std::move(client));
		}
		return id;
	}

	int TcpClientComponent::Send(int id, rpc::Message * message) noexcept
	{
		auto iter = this->mClientMap.find(id);
		if(iter == this->mClientMap.end())
		{
			return XCode::NetWorkError;
		}
		return iter->second->Send(message) ? XCode::Ok : XCode::SendMessageFail;
	}

	void TcpClientComponent::OnSendFailure(int id, rpc::Message* message)
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

	void TcpClientComponent::OnMessage(rpc::Message* message, rpc::Message* response) noexcept
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

	void TcpClientComponent::Remove(int id)
	{
		auto iter = this->mClientMap.find(id);
		if(iter == this->mClientMap.end())
		{
			return;
		}
		iter->second->Close();
		this->mClientMap.erase(iter);
	}

	void TcpClientComponent::OnClientError(int id, int code)
	{
		auto iter = this->mClientMap.find(id);
		if(iter == this->mClientMap.end())
		{
			return;
		}
		this->mClientMap.erase(iter);
	}

	int TcpClientComponent::OnRequest(rpc::Message* message)
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