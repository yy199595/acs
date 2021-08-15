#pragma once

#include "LuaInclude.h"

#define ReadLuaParameter(lua, num) T##num t##num = LuaParameter::Read<T##num>(luaEnv, num);
namespace StaticFunction
{
    template<typename T>
    T &GetStaticFunction(lua_State *lua)
    {
        void *data = lua_touserdata(lua, lua_upvalueindex(1));
        T *func = static_cast<T *>(data);
        return (*func);
    }

    template<typename Ret, typename... Args>
    struct StaticFuncProxy {
    };
}// namespace StaticFunction

// 返回值为 void 类型
namespace StaticFunction
{

    template<>
    struct StaticFuncProxy<void, void> {
        typedef void (*Function)();

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            function();
            return 0;
        }
    };

    template<typename T1>
    struct StaticFuncProxy<void, T1> {
        typedef void (*Function)(T1);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            function(t1);
            return 0;
        }
    };

    template<typename T1, typename T2>
    struct StaticFuncProxy<void, T1, T2> {
        typedef void (*Function)(T1, T2);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);

            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            function(t1, t2);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3>
    struct StaticFuncProxy<void, T1, T2, T3> {
        typedef void (*Function)(T1, T2, T3);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            function(t1, t2, t3);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3, typename T4>
    struct StaticFuncProxy<void, T1, T2, T3, T4> {
        typedef void (*Function)(T1, T2, T3, T4);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            function(t1, t2, t3, t4);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    struct StaticFuncProxy<void, T1, T2, T3, T4, T5> {
        typedef void (*Function)(T1, T2, T3, T4, T5);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            function(t1, t2, t3, t4, t5);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct StaticFuncProxy<void, T1, T2, T3, T4, T5, T6> {
        typedef void (*Function)(T1, T2, T3, T4, T5, T6);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            function(t1, t2, t3, t4, t5, t6);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    struct StaticFuncProxy<void, T1, T2, T3, T4, T5, T6, T7> {
        typedef void (*Function)(T1, T2, T3, T4, T5, T6, T7);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 7);
            function(t1, t2, t3, t4, t5, t6, t7);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    struct StaticFuncProxy<void, T1, T2, T3, T4, T5, T6, T7, T8> {
        typedef void (*Function)(T1, T2, T3, T4, T5, T6, T7, T8);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 7);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 8);
            function(t1, t2, t3, t4, t5, t6, t7, t8);
            return 0;
        }
    };

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    struct StaticFuncProxy<void, T1, T2, T3, T4, T5, T6, T7, T8, T9> {
        typedef void (*Function)(T1, T2, T3, T3, T4, T5, T6, T7, T8, T9);

        static int Invoke(lua_State *luaEnv)
        {
            Function function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 7);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 8);
            T9 t9 = LuaParameter::Read<T9>(luaEnv, 9);
            function(t1, t2, t3, t4, t5, t6, t7, t8, t9);
            return 0;
        }
    };
}// namespace StaticFunction
// 返回值为非void类型
namespace StaticFunction
{
    template<typename Ret>
    struct StaticFuncProxy<Ret> {
        typedef Ret (*Function)();

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            LuaParameter::Write<Ret>(luaEnv, function());
            return 1;
        }
    };

    template<typename Ret, typename T1>
    struct StaticFuncProxy<Ret, T1> {
        typedef Ret (*Function)(T1);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            LuaParameter::Write<Ret>(luaEnv, function(t1));
            return 1;
        }
    };


    template<typename Ret, typename T1, typename T2>
    struct StaticFuncProxy<Ret, T1, T2> {
        typedef Ret (*Function)(T1, T2);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2));
            return 1;
        }
    };


    template<typename Ret, typename T1, typename T2, typename T3>
    struct StaticFuncProxy<Ret, T1, T2, T3> {
        typedef Ret (*Function)(T1, T2, T3);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3));
            return 1;
        }
    };

    template<typename Ret, typename T1, typename T2, typename T3, typename T4>
    struct StaticFuncProxy<Ret, T1, T2, T3, T4> {
        typedef Ret (*Function)(T1, T2, T3, T4);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3, t4));
            return 1;
        }
    };

    template<typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5>
    struct StaticFuncProxy<Ret, T1, T2, T3, T4, T5> {
        typedef Ret (*Function)(T1, T2, T3, T4, T5);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3, t4, t5));
            return 1;
        }
    };

    template<typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct StaticFuncProxy<Ret, T1, T2, T3, T4, T5, T6> {
        typedef Ret (*Function)(T1, T2, T3, T4, T5, T6);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3, t4, t5, t6));
            return 1;
        }
    };

    template<typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    struct StaticFuncProxy<Ret, T1, T2, T3, T4, T5, T6, T7> {
        typedef Ret (*Function)(T1, T2, T3, T4, T5, T6, T7);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 7);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3, t4, t5, t6, t7));
            return 1;
        }
    };

    template<typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    struct StaticFuncProxy<Ret, T1, T2, T3, T4, T5, T6, T7, T8> {
        typedef Ret (*Function)(T1, T2, T3, T4, T5, T6, T7, T8);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 7);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 8);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3, t4, t5, t6, t7, t8));
            return 1;
        }
    };

    template<typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    struct StaticFuncProxy<Ret, T1, T2, T3, T4, T5, T6, T7, T8, T9> {
        typedef Ret (*Function)(T1, T2, T3, T4, T5, T6, T7, T8, T9);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetStaticFunction<Function>(luaEnv);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 1);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 2);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 3);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 4);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 5);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 6);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 7);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 8);
            T9 t9 = LuaParameter::Read<T9>(luaEnv, 9);
            LuaParameter::Write<Ret>(luaEnv, function(t1, t2, t3, t4, t5, t6, t7, t8, t9));
            return 1;
        }
    };
}// namespace StaticFunction