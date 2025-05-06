//
// Created by 64658 on 2025/4/9.
//

#include "LuaNode.h"
#include "Entity/Actor/App.h"
#include "Node/Component/NodeComponent.h"
#include "Cluster/Config/ClusterConfig.h"

namespace lua
{
	int node::AddListen(lua_State* lua)
	{
		acs::Node* server = acs::App::Inst();
		if (lua_isnumber(lua, 1))
		{
			int id = (int)luaL_checkinteger(lua, 1);
			server = acs::App::ActorMgr()->Get(id);
			if (server == nullptr)
			{
				luaL_error(lua, "not find server:%d", id);
				return 0;
			}
		}
		std::string name(luaL_checkstring(lua, 2));
		std::string address(luaL_checkstring(lua, 3));
		lua_pushboolean(lua, server->AddListen(name, address));
		return 1;
	}

	int node::GetListen(lua_State* lua)
	{
		std::string address;
		acs::Node* server = acs::App::Inst();
		if (lua_isinteger(lua, 1))
		{
			int id = (int)luaL_checkinteger(lua, 1);
			server = acs::App::ActorMgr()->Get(id);
		}
		if (server == nullptr)
		{
			return 0;
		}
		std::string name(luaL_checkstring(lua, 2));
		if (!server->GetListen(name, address))
		{
			return 0;
		}
		lua_pushlstring(lua, address.c_str(), address.size());
		return 1;
	}

	int node::AllInfo(lua_State* L)
	{
		acs::Node* server = acs::App::Inst();
		if (lua_isinteger(L, 1))
		{
			int id = (int)luaL_checkinteger(L, 1);
			server = acs::App::ActorMgr()->Get(id);
		}
		if (server == nullptr)
		{
			return 0;
		}
		lua_newtable(L);
		const std::string& name = server->Name();
		lua_pushinteger(L, server->GetNodeId());
		lua_setfield(L, -2, "id");

		lua_pushlstring(L, name.c_str(), name.size());
		lua_setfield(L, -2, "name");

		lua_newtable(L);
		for(const std::pair<std::string, std::string> & item : server->GetListens())
		{
			lua_pushlstring(L, item.second.c_str(), item.second.size());
			lua_setfield(L, -2, item.first.c_str());
		}
		lua_setfield(L, -2, "listen");
		return 1;
	}

	int node::Remove(lua_State* L)
	{
		int id = (int)luaL_checkinteger(L, 1);
		lua_pushboolean(L, acs::App::ActorMgr()->Remove(id));
		return 1;
	}

	int node::Create(lua_State* L)
	{
		int id = (int)luaL_checkinteger(L, 1);
		std::string name = luaL_checkstring(L, 2);
		acs::Node* node = acs::App::ActorMgr()->Get(id);
		if (node == nullptr)
		{
			acs::App::ActorMgr()->Add(std::make_unique<acs::Node>(id, name));
		}
		lua_pushboolean(L, true);
		return 1;
	}

	int node::Query(lua_State* L)
	{
		std::string nodeName;
		std::string service(luaL_checkstring(L, 1));
		if (!acs::ClusterConfig::Inst()->GetServerName(service, nodeName))
		{
			return 0;
		}
		lua_pushlstring(L, nodeName.c_str(), nodeName.size());
		return 1;
	}

	int node::Next(lua_State* L)
	{
		std::string nodeName;
		std::string service(luaL_checkstring(L, 1));
		if (!acs::ClusterConfig::Inst()->GetServerName(service, nodeName))
		{
			luaL_error(L, "[%s] find node name", service.c_str());
			return 0;
		}
		acs::Node * node = acs::App::ActorMgr()->Next(nodeName);
		if(node == nullptr)
		{
			return 0;
		}
		lua_pushinteger(L, node->GetNodeId());
		return 1;
	}

	int node::Rand(lua_State* L)
	{
		std::string nodeName;
		std::string service(luaL_checkstring(L, 1));
		if (!acs::ClusterConfig::Inst()->GetServerName(service, nodeName))
		{
			luaL_error(L, "[%s] find node name", service.c_str());
			return 0;
		}
		acs::Node * node = acs::App::ActorMgr()->Rand(nodeName);
		if(node == nullptr)
		{
			return 0;
		}
		lua_pushinteger(L, node->GetNodeId());
		return 1;
	}

	int node::Hash(lua_State* L)
	{
		std::string nodeName;
		std::string service(luaL_checkstring(L, 1));
		if (!acs::ClusterConfig::Inst()->GetServerName(service, nodeName))
		{
			luaL_error(L, "[%s] find node name", service.c_str());
			return 0;
		}
		long long hash = luaL_checkinteger(L, 2);
		acs::Node * node = acs::App::ActorMgr()->Hash(nodeName, hash);
		if(node == nullptr)
		{
			return 0;
		}
		lua_pushinteger(L, node->GetNodeId());
		return 1;
	}
}