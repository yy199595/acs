//
// Created by zmhy0073 on 2022/6/6.
//

#include"LuaMysql.h"
#include"Script/Extension/Json/Json.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#include<Component/Scene/MessageComponent.h>
#include"Component/Mysql/MysqlAgentComponent.h"

using namespace Sentry;
namespace Lua
{
    int Mysql::Add(lua_State *lua)
    {
        lua_pushthread(lua);
        MysqlAgentComponent *mysqlComponent = UserDataParameter::Read<MysqlAgentComponent *>(lua, 1);
        std::shared_ptr<Message> message = UserDataParameter::Read<std::shared_ptr<Message>>(lua, 2);

        TaskComponent *taskComponent = App::Get()->GetTaskComponent();
        std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));

        taskComponent->Start([luaRpcTaskSource, mysqlComponent, message]() {
            XCode code = mysqlComponent->Add(*message);
            luaRpcTaskSource->SetResult(code);
        });
        return luaRpcTaskSource->Await();
    }

    int Mysql::Delete(lua_State *lua)
    {
        lua_pushthread(lua);
        MysqlAgentComponent *mysqlComponent = UserDataParameter::Read<MysqlAgentComponent *>(lua, 1);

        if(!lua_isstring(lua, 2))
        {
            luaL_error(lua, "must be string");
            return 0;
        }
        if(!lua_istable(lua, 3))
        {
            luaL_error(lua, "must be table");
            return 0;
        }
        size_t size = 0;
        std::string json;
        std::string table(lua_tolstring(lua, 2, &size), size);
        Lua::Json::Read(lua, 3, &json);

        TaskComponent *taskComponent = App::Get()->GetTaskComponent();
        std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));

        taskComponent->Start([luaRpcTaskSource, mysqlComponent, json, table]()
        {
           XCode code = mysqlComponent->Delete(table, json);
           luaRpcTaskSource->SetResult(code);
        });
        return luaRpcTaskSource->Await();
    }

    int Mysql::Save(lua_State *lua)
    {
        lua_pushthread(lua);
        MysqlAgentComponent *mysqlComponent = UserDataParameter::Read<MysqlAgentComponent *>(lua, 1);
        std::shared_ptr<Message> message = UserDataParameter::Read<std::shared_ptr<Message>>(lua, 2);

        TaskComponent *taskComponent = App::Get()->GetTaskComponent();
        std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));

        taskComponent->Start([luaRpcTaskSource, mysqlComponent, message]()
        {
            XCode code = mysqlComponent->Save(*message);
            luaRpcTaskSource->SetResult(code);
        });
        return luaRpcTaskSource->Await();
    }

    int Mysql::Update(lua_State *lua)
    {
        lua_pushthread(lua);
        MysqlAgentComponent *mysqlComponent = UserDataParameter::Read<MysqlAgentComponent *>(lua, 1);

        std::string whereJson, updateJson;
        std::string table(lua_tostring(lua, 2));
        Lua::Json::Read(lua, 3, &updateJson);
        Lua::Json::Read(lua, 4, &whereJson);

        TaskComponent *taskComponent = App::Get()->GetTaskComponent();
        std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));

        taskComponent->Start([luaRpcTaskSource, mysqlComponent, table, whereJson, updateJson]()
        {
            XCode code = mysqlComponent->Update(table, updateJson, whereJson);
            luaRpcTaskSource->SetResult(code);
        });
        return luaRpcTaskSource->Await();
    }

    int Mysql::Query(lua_State *lua)
    {
        lua_pushthread(lua);
        MysqlAgentComponent *mysqlComponent = UserDataParameter::Read<MysqlAgentComponent *>(lua, 1);

        std::string whereJson;
        std::string table(lua_tostring(lua, 2));
        Lua::Json::Read(lua, 3, &whereJson);
        TaskComponent *taskComponent = App::Get()->GetTaskComponent();
        std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));
        MessageComponent *messageComponent = App::Get()->GetComponent<MessageComponent>();
        std::shared_ptr<Message> response = messageComponent->New(table);
        if(response == nullptr)
        {
            luaL_error(lua, "create message [%s] error", table.c_str());
            return 0;
        }

        taskComponent->Start([mysqlComponent, whereJson, luaRpcTaskSource, response]()
        {
            XCode code = mysqlComponent->QueryOnce(whereJson, response);
            luaRpcTaskSource->SetResult(code, response);
        });
        return luaRpcTaskSource->Await();
    }

    int Mysql::QueryOnce(lua_State *lua)
    {

    }
}