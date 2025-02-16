//
// Created by leyi on 2023/6/29.
//
#ifdef __OS_WIN__
#include <codecvt>
#endif
#include "LuaFile.h"
#include "Util/File/FileHelper.h"
#include "Util/File/DirectoryHelper.h"
namespace lua
{
	int LuaFile::Write(lua_State * L)
	{
		size_t size = 0;
		std::ofstream ofs;
		const std::string path(luaL_checkstring(L, 1));
		const char * content = luaL_checklstring(L, 2, &size);
#ifdef __OS_WIN__
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring newPath = converter.from_bytes(path);
		ofs.open(newPath, std::ios::ate);
#else
		ofs.open(path, std::ios::ate);
#endif
		if(!ofs.is_open())
		{
			return 0;
		}
		ofs.write(content, size);
		ofs.flush();
		ofs.close();
		lua_pushinteger(L, size);
		return 1;
	}

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
			std::string & path = files[index];
			lua_pushstring(lua, path.c_str());
			lua_seti(lua, -2, index + 1);
		}
		return 1;
	}

	int LuaFile::GetLastWriteTime(lua_State* lua)
	{
		std::string path(luaL_checkstring(lua, 1));
		long long time = help::fs::GetLastWriteTime(path);
		lua_pushinteger(lua, time);
		return 1;
	}

	int LuaDir::Make(lua_State* lua)
	{
		std::string dir(luaL_checkstring(lua, 1));
		if(help::dir::DirectorIsExist(dir))
		{
			lua_pushboolean(lua, true);
			return 1;
		}
		lua_pushboolean(lua, help::dir::MakeDir(dir));
		return 1;
	}

	int LuaDir::IsExist(lua_State* lua)
	{
		std::string dir(luaL_checkstring(lua, 1));
		lua_pushboolean(lua, help::dir::DirectorIsExist(dir));
		return 1;
	}
}
