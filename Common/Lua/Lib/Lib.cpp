//
// Created by yy on 2025/1/15.
//
#include "Lib.h"
#include "Redis/Lua/LuaRedis.h"
#include "Util/File/Lua/LuaFile.h"
#include "Auth/Lua/Auth.h"
#include "Util/Crypt/LuaMd5.h"
#include "Entity/Lua/LuaActor.h"
#include "Yyjson/Lua/ljson.h"
#include "Proto/Lua/Bson.h"
#include "Mongo/Lua/LuaMongo.h"
#include "Sqlite/Lua/LuaSqlite.h"
#include "Timer/Lua/Timer.h"
#include "Log/Lua/LuaLogger.h"
#include "Http/Lua/LuaHttp.h"
#include "Proto/Lua/Message.h"
#include "Oss/Lua/LuaOss.h"

namespace lua
{
	int lib::luaopen_lfs(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Find", lua::LuaFile::Find },
				{ "GetMd5", lua::LuaFile::GetMd5 },
				{ "IsExist", lua::LuaFile::IsExist },
				{ "GetFiles", lua::LuaFile::GetFiles },
				{ "GetFileName", lua::LuaFile::GetFileName },
				{ "GetLastWriteTime", lua::LuaFile::GetLastWriteTime },

				{ "MakeDir", lua::LuaDir::Make },
				{ "IsExistDir", lua::LuaDir::IsExist },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_loss(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "sign", oss::Sign },
				{ "upload", oss::Upload },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lredisdb(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Sub", lua::redis::Sub },
				{ "Run", lua::redis::Run },
				{ "Send", lua::redis::Send },
				{ "Send", lua::redis::UnSub },
				{ "Call", lua::redis::Call },
				{ "SyncRun", lua::redis::SyncRun },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}


namespace lua
{
	int lib::luaopen_ljwt(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "encode", lua::ljwt::Create },
				{ "decode", lua::ljwt::Verify },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lfmt(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "format", lua::lfmt::format },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lmd5(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "ToString", lua::md5::ToString },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}

	int lib::luaopen_lbase64(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "encode", lua::base64::Encode },
				{ "decode", lua::base64::Decode },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lproto(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "new", lua::MessageEx::New },
				{ "encode", lua::MessageEx::Encode },
				{ "decode", lua::MessageEx::Decode },
				{ "import", lua::MessageEx::Import },
				{ "to_json", lua::MessageEx::ToJson },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lapp(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Stop", acs::LuaActor::Stop },
				{ "Send", acs::LuaActor::Send },
				{ "Call", acs::LuaActor::Call },
				{ "Random", acs::LuaActor::Random },
				{ "GetPath", acs::LuaActor::GetPath },
				{ "NewGuid", acs::LuaActor::NewGuid },
				{ "NewUuid", acs::LuaActor::NewUuid },
				{ "Publish", acs::LuaActor::Publish },
				{ "AddAddr", acs::LuaPlayer::AddAddr },
				{ "AddListen", acs::LuaActor::AddListen },
				{ "GetListen", acs::LuaActor::GetListen },
				{ "GetConfig", acs::LuaActor::GetConfig },
				{ "GetServers", acs::LuaActor::GetServers },
				{ "MakeServer", acs::LuaActor::MakeServer },
				{ "HasComponent", acs::LuaActor::HasComponent },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_ljson(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "read", yyjson::read_file },
				{ "encode", yyjson::encode },
				{ "decode", yyjson::decode },
				{ "pretty", yyjson::pretty },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_llog(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Output", lua::Log::Output },
				{ "Show", lua::Console::Show },

				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lbson(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "encode", lbson::encode },
				{ "decode", lbson::decode },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lhttp(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Do", lua::HttpClient::Do },
				{ "Get", lua::HttpClient::Get },
				{ "Post", lua::HttpClient::Post },
				{ "Upload", lua::HttpClient::Upload },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lmonogodb(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Run", lua::LuaMongo::Run },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_lsqlitedb(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Exec", lua::Sqlite::Exec },
				{ "Find", lua::Sqlite::Find },
				{ "FindOne", lua::Sqlite::FindOne },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
	int lib::luaopen_ltimer(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "Add", lua::Timer::Add },
				{ "Remove", lua::Timer::Remove },
				{ "AddUpdate", lua::Timer::AddUpdate },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}