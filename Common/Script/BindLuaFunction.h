#pragma once

#include <Define/CommonTypeDef.h>
#include <Script/LuaInclude.h>

namespace Sentry
{
    class GameObject;

    class BindLuaFunction
    {
    public:
        BindLuaFunction(lua_State *lua, int ref)
            : mRef(ref), mLua(lua) {}

        ~BindLuaFunction() { luaL_unref(mLua, LUA_REGISTRYINDEX, this->mRef); }

    public:
        virtual bool Invoke(GameObject *gameObject, const std::string &data);

    protected:
        const int mRef;
        lua_State *mLua;
    };
}// namespace Sentry

namespace Sentry
{
    class BindServerLuaFunction : public BindLuaFunction
    {
    public:
        BindServerLuaFunction(lua_State *lua, int ref) : BindLuaFunction(lua, ref) {}

        ~BindServerLuaFunction() { luaL_unref(mLua, LUA_REGISTRYINDEX, this->mRef); }
    };
}// namespace Sentry

namespace Sentry
{
    class BindClientLuaFunction : public BindLuaFunction
    {
    public:
        BindClientLuaFunction(lua_State *lua, int ref, std::string name) : BindLuaFunction(lua, ref), mPbName(name) {}

        ~BindClientLuaFunction() { luaL_unref(mLua, LUA_REGISTRYINDEX, this->mRef); }

    public:
        bool Invoke(GameObject *gameObject, const std::string &data) override;

    private:
        const std::string mPbName;
    };
}// namespace Sentry