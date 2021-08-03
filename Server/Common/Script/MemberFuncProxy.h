
#pragma once

#include"LuaInclude.h"
#include<functional>

namespace MemberFunction
{
    template<typename T>
    T &GetMemberFunction(lua_State *lua)
    {
        void *ptr = lua_touserdata(lua, lua_upvalueindex(1));
        T *func = static_cast<T *>(ptr);
        return (*func);
    }

    template<typename T, typename Ret, typename ...Args>
    struct MemberFuncProxy
    {
    };
}
// 返回值为void类型
namespace MemberFunction
{
    template<typename T>
    struct MemberFuncProxy<T, void>
    {
        typedef void(T::*Function)();

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            (ptr->*function)();
            return 0;
        }
    };

    template<typename T, typename T1>
    struct MemberFuncProxy<T, void, T1>
    {
        typedef void(T::*Function)(T1);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            (ptr->*function)(t1);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2>
    struct MemberFuncProxy<T, void, T1, T2>
    {
        typedef void(T::*Function)(T1, T2);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            (ptr->*function)(t1, t2);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3>
    struct MemberFuncProxy<T, void, T1, T2, T3>
    {
        typedef void(T::*Function)(T1, T2, T3);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            (ptr->*function)(t1, t2, t3);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3, typename T4>
    struct MemberFuncProxy<T, void, T1, T2, T3, T4>
    {
        typedef void(T::*Function)(T1, T2, T3, T4);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            (ptr->*function)(t1, t2, t3, t4);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
    struct MemberFuncProxy<T, void, T1, T2, T3, T4, T5>
    {
        typedef void(T::*Function)(T1, T2, T3, T4, T5);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            (ptr->*function)(t1, t2, t3, t4, t5);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct MemberFuncProxy<T, void, T1, T2, T3, T4, T5, T6>
    {
        typedef void(T::*Function)(T1, T2, T3, T4, T5, T6);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            (ptr->*function)(t1, t2, t3, t4, t5, t6);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    struct MemberFuncProxy<T, void, T1, T2, T3, T4, T5, T6, T7>
    {
        typedef void(T::*Function)(T1, T2, T3, T4, T5, T6, T7);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 8);
            (ptr->*function)(t1, t2, t3, t4, t5, t6, t7);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    struct MemberFuncProxy<T, void, T1, T2, T3, T4, T5, T6, T7, T8>
    {
        typedef void(T::*Function)(T1, T2, T3, T4, T5, T6, T7, T8);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 8);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 9);
            (ptr->*function)(t1, t2, t3, t4, t5, t6, t7, t8);
            return 0;
        }
    };

    template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    struct MemberFuncProxy<T, void, T1, T2, T3, T4, T5, T6, T7, T8, T9>
    {
        typedef void(T::*Function)(T1, T2, T3, T4, T5, T6, T7, T8, T9);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 8);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 9);
            T9 t9 = LuaParameter::Read<T9>(luaEnv, 10);
            (ptr->*function)(t1, t2, t3, t4, t5, t6, t7, t8, t9);
            return 0;
        }
    };
}
// 返回值为非void类型
namespace MemberFunction
{
    template<typename T, typename Ret>
    struct MemberFuncProxy<T, Ret>
    {
        typedef Ret(T::*Function)();

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)());
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1>
    struct MemberFuncProxy<T, Ret, T1>
    {
        typedef Ret(T::*Function)(T1);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1));
            return 1;
        }
    };


    template<typename T, typename Ret, typename T1, typename T2>
    struct MemberFuncProxy<T, Ret, T1, T2>
    {
        typedef Ret(T::*Function)(T1, T2);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2));
            return 1;
        }
    };


    template<typename T, typename Ret, typename T1, typename T2, typename T3>
    struct MemberFuncProxy<T, Ret, T1, T2, T3>
    {
        typedef Ret(T::*Function)(T1, T2, T3);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3));
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1, typename T2, typename T3, typename T4>
    struct MemberFuncProxy<T, Ret, T1, T2, T3, T4>
    {
        typedef Ret(T::*Function)(T1, T2, T3, T4);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3, t4));
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5>
    struct MemberFuncProxy<T, Ret, T1, T2, T3, T4, T5>
    {
        typedef Ret(T::*Function)(T1, T2, T3, T4, T5);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3, t4, t5));
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct MemberFuncProxy<T, Ret, T1, T2, T3, T4, T5, T6>
    {
        typedef Ret(T::*Function)(T1, T2, T3, T4, T5, T6);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3, t4, t5, t6));
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    struct MemberFuncProxy<T, Ret, T1, T2, T3, T4, T5, T6, T7>
    {
        typedef Ret(T::*Function)(T1, T2, T3, T4, T5, T6, T7);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 8);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3, t4, t5, t6, t7));
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    struct MemberFuncProxy<T, Ret, T1, T2, T3, T4, T5, T6, T7, T8>
    {
        typedef Ret(T::*Function)(T1, T2, T3, T4, T5, T6, T7, T8);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 8);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 9);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3, t4, t5, t6, t7, t8));
            return 1;
        }
    };

    template<typename T, typename Ret, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    struct MemberFuncProxy<T, Ret, T1, T2, T3, T4, T5, T6, T7, T8, T9>
    {
        typedef Ret(T::*Function)(T1, T2, T3, T4, T5, T6, T7, T8, T9);

        static int Invoke(lua_State *luaEnv)
        {
            Function &function = GetMemberFunction<Function>(luaEnv);
            T *ptr = PtrProxy<T>::Read(luaEnv, 1);
            T1 t1 = LuaParameter::Read<T1>(luaEnv, 2);
            T2 t2 = LuaParameter::Read<T2>(luaEnv, 3);
            T3 t3 = LuaParameter::Read<T3>(luaEnv, 4);
            T4 t4 = LuaParameter::Read<T4>(luaEnv, 5);
            T5 t5 = LuaParameter::Read<T5>(luaEnv, 6);
            T6 t6 = LuaParameter::Read<T6>(luaEnv, 7);
            T7 t7 = LuaParameter::Read<T7>(luaEnv, 8);
            T8 t8 = LuaParameter::Read<T8>(luaEnv, 9);
            T9 t9 = LuaParameter::Read<T9>(luaEnv, 10);
            LuaParameter::Write<Ret>(luaEnv, (ptr->*function)(t1, t2, t3, t4, t5, t6, t7, t8, t9));
            return 1;
        }
    };
}
