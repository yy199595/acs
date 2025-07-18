//
// Created by yy on 2025/1/15.
//
#include "Lib.h"
#include "Redis/Lua/LuaRedis.h"
#include "Util/File/Lua/LuaFile.h"
#include "Auth/Lua/Auth.h"
#include "Util/Crypt/LuaCryptLib.h"
#include "Entity/Lua/LuaActor.h"
#include "Yyjson/Lua/ljson.h"
#include "Proto/Lua/Bson.h"
#include "Mongo/Lua/LuaMongo.h"
#include "Sqlite/Lua/LuaSqlite.h"
#include "Timer/Lua/Timer.h"
#include "Log/Lua/LuaLogger.h"
#include "Http/Lua/LuaHttp.h"
#include "Proto/Lua/Message.h"
#include "AliCloud/Lua/LuaOss.h"
#include "Net/Lua/LuaSocket.h"
#include "Util/Ssl/LuaRsa.h"
#include "Auth/Lua/LuaAes.h"
#include "Util/Tools/Guid.h"
#include "Core/Excel/excel.h"
#include "Util/Zip/Zip.h"
#include "Mysql/Lua/LuaMysql.h"
#include "Pgsql/Lua/LuaPgsql.h"
#include "Node/Lua/LuaNode.h"
#include "Router/Lua/LuaRouter.h"
#include "Util/Censor/LuaCensor.h"
namespace lua
{

    int guid::New(lua_State* L)
    {
        int id = 0;
        int count = 0;
        if (lua_isinteger(L, 1))
        {
            id = lua_tointeger(L, 1);
        }
        if (lua_isinteger(L, 2))
        {
            count = lua_tointeger(L, 2);
        }
        long long number = help::ID::Create(id);
        if (count > 0)
        {
            int num = 1;
            for (int index = 0; index < count; index++)
            {
                num *= 10;
            }
            number = number % num;
        }
        lua_pushinteger(L, number);
        return 1;
    }
}

namespace lua
{
    int lib::luaopen_lfs(lua_State* L)
    {
        luaL_Reg l[] = {
            {"Find", lua::LuaFile::Find},
            {"Write", lua::LuaFile::Write},
            {"IsExist", lua::LuaFile::IsExist},
            {"GetFiles", lua::LuaFile::GetFiles},
            {"GetFileName", lua::LuaFile::GetFileName},
            {"GetLastWriteTime", lua::LuaFile::GetLastWriteTime},

            {"MakeDir", lua::LuaDir::Make},
            {"IsExistDir", lua::LuaDir::IsExist},
            {nullptr, nullptr}
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
            {"run", lua::redis::Run},
            {"send", lua::redis::Send},
            {"call", lua::redis::Call},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }

	int lib::luaopen_lsub_redisdb(lua_State* L)
	{
		luaL_Reg l[] = {
				{"run", lua::sub_redis::Run},
				{nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}

namespace lua
{
    int lib::luaopen_lmysqldb(lua_State* L)
    {
        luaL_Reg l[]{
            {"run", lua::lmysql::Run},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
    int lib::luaopen_lpgsqldb(lua_State* L)
    {
        luaL_Reg l[]{
            {"run", lua::lpgsql::Run},
            {nullptr, nullptr}
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
            {"format", lua::lfmt::format},
            {nullptr, nullptr}
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
            {"hex", lua::md5::Hex},
            {"hash", lua::md5::ToString},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }

    int lib::luaopen_lbase64(lua_State* L)
    {
        luaL_Reg l[] = {
            {"encode", lua::base64::Encode},
            {"decode", lua::base64::Decode},
            {nullptr, nullptr}
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
            {"new", lua::MessageEx::New},
            {"encode", lua::MessageEx::Encode},
            {"decode", lua::MessageEx::Decode},
            {"import", lua::MessageEx::Import},
            {"to_json", lua::MessageEx::ToJson},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
    int lib::luaopen_lapp(lua_State* L)
    {
        luaL_Reg l[] = {
            {"Stop", acs::LuaActor::Stop},
            {"GetPath", acs::LuaApp::GetPath},
            {"NewGuid", acs::LuaApp::NewGuid},
            {"NewUuid", acs::LuaApp::NewUuid},
            {"GetConfig", acs::LuaApp::GetConfig},
            {"HasComponent", acs::LuaApp::HasComponent},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }

	int lib::luaopen_lactor(lua_State* L)
	{
		luaL_Reg l[] = {
				{"Send", acs::LuaActor::Send},
				{"Call", acs::LuaActor::Call},
				{"Broadcast", acs::LuaActor::Broadcast},
				{nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}

	int lib::luaopen_lnode(lua_State* L)
	{
		luaL_Reg l[] = {
				{"next", lua::node::Next },
				{"rand", lua::node::Rand },
				{"hash", lua::node::Hash },
				{"query", lua::node::Query },
				{"create", lua::node::Create },
				{"remove", lua::node::Remove },
				{"get_info", lua::node::AllInfo },
				{"get_listen", lua::node::GetListen },
				{"add_listen", lua::node::AddListen },
				{"get_nodes", acs::LuaActor::GetServers},
				{nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}

	int lib::luaopen_lrouter(lua_State* L)
	{
		luaL_Reg l[] = {
				{"send", acs::LuaRouter::Send},
				{"call", acs::LuaRouter::Call},
				{nullptr, nullptr}
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
            {"read", yyjson::read_file},
            {"encode", yyjson::encode},
            {"decode", yyjson::decode},
            {"pretty", yyjson::pretty},
			{"new", lua::ljson::create},

            {nullptr, nullptr}
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
            {"Output", lua::Log::Output},
            {"Show", lua::Console::Show},

            {nullptr, nullptr}
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
            {"encode", lbson::encode},
            {"decode", lbson::decode},
            {nullptr, nullptr}
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
            {"Do", lua::HttpClient::Do},
            {"Get", lua::HttpClient::Get},
            {"Post", lua::HttpClient::Post},
            {"Upload", lua::HttpClient::Upload},
            {nullptr, nullptr}
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
            {"run", lua::LuaMongo::Run},
            {nullptr, nullptr}
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
            {"set", lua::Sqlite::Set},
            {"get", lua::Sqlite::Get},
            {"del", lua::Sqlite::Del},
            {"run", lua::Sqlite::Run},
			{"build", lua::Sqlite::Build},
			{"invoke", lua::Sqlite::Invoke},
            {"set_timeout", lua::Sqlite::SetTimeout},
            {nullptr, nullptr}
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
            {"timeout", lua::Timer::Add},
            {"cancel", lua::Timer::Remove},
            {"update", lua::Timer::AddUpdate},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
    int lib::luaopen_ltcp(lua_State* L)
    {
        luaL_Reg l[] = {
            {"query", lua::TcpSock::Query},
            {"listen", lua::TcpSock::Listen},
            {"connect", lua::TcpSock::Connect},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
	int lib::luaopen_lcensor(lua_State * L)
	{
		luaL_Reg l[] = {
				{"new", censor::New},
				{nullptr, nullptr}
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
            {"encode", lua::ljwt::Create},
            {"decode", lua::ljwt::Verify},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

#ifdef __ENABLE_OPEN_SSL__
namespace lua
{
    int lib::luaopen_lrsa(lua_State* L)
    {
        luaL_Reg l[] = {
            {"create", lua::rsa::Init},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
    int lib::luaopen_lraes(lua_State* L)
    {
        luaL_Reg l[] = {
            {"encode", lua::laes::Encode},
            {"decode", lua::laes::Decode},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}


#endif

namespace lua
{
    int lib::luaopen_loss(lua_State* L)
    {
        luaL_Reg l[] = {
            {"sign", oss::Sign},
            {"upload", oss::Upload},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
    int lib::luaopen_lsha1(lua_State* L)
    {
        luaL_Reg l[] = {
            {"xor", lua::sha1::Xor},
            {"hash", lua::sha1::Hash},
            {"hmac", lua::sha1::HMac},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}


namespace lua
{
    int lib::luaopen_lguid(lua_State* L)
    {
        luaL_Reg l[] = {
            {"new", lua::guid::New},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}


namespace lua
{
    int excel::New(lua_State* L)
    {
        const char* path = luaL_checkstring(L, 1);
        std::unique_ptr<lxlsx::ExcelFile> excelFile = std::make_unique<lxlsx::ExcelFile>();
        {
            std::string error;
            if (!excelFile->Open(path, error))
            {
                luaL_error(L, "[%s] => %s", path, error.c_str());
                return 0;
            }
            Lua::UserDataParameter::Write(L, excelFile.release());
        }
        return 1;
    }

    int lib::luaopen_lexcel(lua_State* L)
    {
        luaL_Reg l[] = {
            {"new", lua::excel::New},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}

namespace lua
{
    namespace zip
    {
        inline int Create(lua_State* L)
        {
            bool result = true;
            std::string zipPath;
            std::string fileDir(luaL_checkstring(L, 1));
            switch (lua_type(L, 2))
            {
            case LUA_TTABLE:
                {
                    if (lua_rawlen(L, 2) <= 0)
                    {
                        luaL_error(L, "file is empty");
                        return 0;
                    }
                    lua_pushnil(L);
                    std::vector<std::string> files;
                    while (lua_next(L, 2) != 0)
                    {
                        files.emplace_back(luaL_checkstring(L, -1));
                        lua_pop(L, 1);
                    }
                    zipPath.append(luaL_checkstring(L, 3));
                    result = help::zip::Create(fileDir, files, zipPath);
                    break;
                }
            case LUA_TSTRING:
                zipPath.append(luaL_checkstring(L, 2));
                result = help::zip::Create(fileDir, zipPath);
                break;
            default:
                luaL_error(L, "args:2 is string or array");
                return 0;
            }
            lua_pushboolean(L, result);
            return 1;
        }

        inline int UnZip(lua_State* L)
        {
            std::string outDir(luaL_checkstring(L, 1));
            std::string zipPath(luaL_checkstring(L, 2));
            bool result = help::zip::Unzip(outDir, zipPath);
            lua_pushboolean(L, result);
            return 1;
        }
    }

    int lib::luaopen_lzip(lua_State* L)
    {
        luaL_Reg l[] = {
            {"unzip", lua::zip::UnZip},
            {"create", lua::zip::Create},
            {nullptr, nullptr}
        };
        luaL_newlib(L, l);
        return 1;
    }
}
