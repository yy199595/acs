//
// Created by leyi on 2023/6/29.
//

#include "LuaFile.h"
#include "fstream"
#include "Util/Crypt/MD5.h"
#include "Util/File/FileHelper.h"
#include "Util/File/DirectoryHelper.h"
namespace Lua
{
	int LuaFile::GetFileName(lua_State* lua)
	{
		std::string path(luaL_checkstring(lua, 1));
		size_t pos = path.find_last_of('/');
		if(pos == std::string::npos)
		{
			lua_pushstring(lua, path.c_str());
			return 1;
		}
		std::string fileName = path.substr(pos + 1);
		lua_pushlstring(lua, fileName.c_str(), fileName.size());
		return 1;
	}

	int LuaFile::Find(lua_State* lua)
	{
		std::vector<std::string> files;
		std::string path(luaL_checkstring(lua, 1));
		std::string type(luaL_checkstring(lua, 2));
		if(!help::dir::GetFilePaths(path, type, files))
		{
			return 0;
		}
		lua_newtable(lua);
		{
			for(size_t index = 0; index < files.size(); index++)
			{
				lua_pushstring(lua, files[index].c_str());
				lua_seti(lua, -2, index + 1);
			}
		}
		return 1;
	}

	int LuaFile::IsExist(lua_State* lua)
	{
		std::string path(luaL_checkstring(lua, 1));
		bool ret = help::fs::FileIsExist(path);
		lua_pushboolean(lua, ret);
		return 1;
	}

	int LuaFile::GetFiles(lua_State* lua)
	{
		std::vector<std::string> files;
		std::string dir(luaL_checkstring(lua, 1));
		if(lua_isstring(lua, 2))
		{
			std::string type(luaL_checkstring(lua, 2));
			if(!help::dir::GetFilePaths(dir, type, files))
			{
				return 0;
			}
		}
		else
		{
			if(!help::dir::GetFilePaths(dir, files))
			{
				return 0;
			}
		}

		lua_createtable(lua, 0, files.size());
		for(size_t index = 0; index < files.size(); index++)
		{
			const std::string & path = files[index];
			lua_pushstring(lua, path.c_str());
			lua_seti(lua, -2, index + 1);
		}
		return 1;
	}

	int LuaFile::GetMd5(lua_State* lua)
	{
		std::string path(luaL_checkstring(lua, 1));
		std::ifstream fs(path, std::ios::in);
		if(fs.is_open())
		{
			MD5 md5(fs);
			const std::string str = md5.toString();
			lua_pushlstring(lua, str.c_str(), str.size());
			return 1;
		}
		return 0;
	}

	int LuaFile::GetLastWriteTime(lua_State* lua)
	{
		std::string path(luaL_checkstring(lua, 1));
		long long time = help::fs::GetLastWriteTime(path);
		lua_pushinteger(lua, time);
		return 1;
	}
}