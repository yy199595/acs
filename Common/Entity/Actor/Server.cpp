//
// Created by leyi on 2023/5/23.
//

#include"Server.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/Define.h"
namespace Tendo
{
	Server::Server(int id, const std::string & name)
		: Actor(id), mName(name), mRpc("rpc")
	{

	}

	bool Server::OnInit()
	{
		if (this->mListens.count(this->mRpc) == 0)
		{
			LOG_ERROR("not rpc address " << this->Name());
			return false;
		}
		return true;
	}

	int Server::GetAddress(const std::string& func, std::string& addr)
	{
		auto iter = this->mListens.find(this->mRpc);
		if(iter == this->mListens.end())
		{
			return XCode::AddressAllotFailure;
		}
		addr = iter->second;
		return XCode::Successful;
	}

	void Server::AddListen(const std::string& name, const std::string& addr)
	{
		if (addr.empty() || name.empty())
		{
			return;
		}
		this->mListens[name] = addr;
	}

	bool Server::GetListen(const std::string& name, std::string& addr)
	{
		auto iter = this->mListens.find(name);
		if(iter == this->mListens.end())
		{
			return false;
		}
		addr = iter->second;
		return true;
	}

	int Server::GetListenEx(lua_State* lua)
	{
		std::string address;
		long long id = luaL_checkinteger(lua, 1);
		const std::string name(luaL_checkstring(lua, 2));
		Server * server = App::Inst()->ActorMgr()->GetServer(id);
		if(server == nullptr || !server->GetListen(name, address))
		{
			return 0;
		}
		lua_pushlstring(lua, address.c_str(), address.size());
		return 1;
	}
}