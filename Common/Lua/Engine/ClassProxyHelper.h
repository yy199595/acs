
#pragma once

#include<cstring>
#include<typeinfo>
#include"CtorFunction.h"
#include"ClassMateProxy.h"
#include"MemberFuncProxy.h"
#include"StaticFuncProxy.h"
#include "ValueProxy.h"
namespace Lua
{
	class ClassProxyHelper
	{
	 public:
		ClassProxyHelper(lua_State * lua, const std::string & name);
	 public:
		template<typename T>
		void BeginRegister();

		template<typename T>
		void PushObject(T * obj);

		template<typename T, typename ... Args>
		void PushCtor();

		template<typename T, typename Base>
		void PushBaseClass();

		template<typename T>
		void PushMember(const char * name, const T & value);

		template<typename T, typename V>
		void PushMemberField(const char * name, V T::*member);

		template<typename T, typename Ret, typename ... Args>
		void PushMemberFunction(const char* name, Ret(T::*func)(Args ...));

		template<typename T, typename Ret, typename ... Args>
		void PushMemberFunction(const char* name, Ret(T::*func)(Args ...) const);

		template<typename T, typename BC, typename Ret, typename ... Args>
		void PushBaseMemberFunction(const char* name, Ret(BC::*func)(Args ...));

		void PushExtensionFunction(const char* name, lua_CFunction func);

		void PushGlobalExtensionFunction(const char* name, lua_CFunction func);

	 public:

		void BeginNewTable();

		void BeginNewTable(const std::string & name);

		ClassProxyHelper Clone(const std::string & name);

		template<typename Ret, typename ... Args>
		void PushStaticFunction(const char* name, Ret(* func)(Args ...));

	 private:
		lua_State * mLua;
		std::string mName;
	};

	template<typename T, typename V>
	void ClassProxyHelper::PushMemberField(const char* name, V T::*member)
	{
		typedef V T::*Member;
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			char buff[100] = { 0 };
			sprintf(buff, "get_%s", name);
			lua_pushstring(this->mLua, buff);
			new(lua_newuserdata(this->mLua, sizeof(Member))) Member(member);
			lua_pushcclosure(this->mLua, ValueProxy<T, V>::Get, 1);
			lua_settable(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	template<typename T, typename ... Args>
	void ClassProxyHelper::PushCtor()
	{
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, "New");
			lua_pushcclosure(this->mLua, CtorFunction::PushCtor<T, Args ...>, 0);
			lua_settable(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	template<typename T, typename Base>
	inline void ClassProxyHelper::PushBaseClass()
	{
		if (!ClassNameProxy::HasRegisterClass<Base>())
		{
			printf("[Lua Error] : The parent class is not registered %s\n", typeid(Base).name());
			return;
		}
		if (!std::is_base_of<Base, T>::value)
		{
			printf("[Lua Error] : T Is not a subclass %s\n", typeid(Base).name());
			return;
		}
		ClassNameProxy::OnPushParent<T, Base>();
	}

	template<typename T>
	void ClassProxyHelper::BeginRegister()
	{
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_isnil(this->mLua, -1))
		{
			lua_newtable(this->mLua);
			{
				lua_pushstring(this->mLua, "__index");
				lua_pushcclosure(this->mLua, ClassMateProxy::OnMateTableGet<T>, 0);
				lua_rawset(this->mLua, -3);

				lua_pushstring(this->mLua, "__gc");
				lua_pushcclosure(this->mLua, ClassMateProxy::OnDestroy<T>, 0);
				lua_rawset(this->mLua, -3);

				lua_pushstring(this->mLua, "__name");
				lua_pushstring(this->mLua, this->mName.c_str());
				lua_rawset(this->mLua, -3);

			}
			lua_setglobal(this->mLua, this->mName.c_str());
		}
		ClassNameProxy::OnClassRegister<T>(this->mName);
	}

	inline void ClassProxyHelper::PushExtensionFunction(const char* funcName, lua_CFunction func)
	{
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, funcName);
			lua_pushcfunction(this->mLua, func);
			lua_rawset(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	template<typename T, typename Ret, typename ... Args>
	void ClassProxyHelper::PushMemberFunction(const char* name, Ret(T::*func)(Args ...))
	{
		const char* className = ClassNameProxy::GetLuaClassName<T>();
		typedef Ret(T::*MemberFunctionType)(Args ...);
		lua_getglobal(this->mLua, className);
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, name);
			new(lua_newuserdata(this->mLua, sizeof(MemberFunctionType))) MemberFunctionType(func);
			lua_pushcclosure(this->mLua, MemberFunction::MemberFuncProxy<T, Ret, Args...>::Invoke, 1);
			lua_settable(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	template<typename T, typename Ret, typename ...Args>
	inline void ClassProxyHelper::PushMemberFunction(const char* name, Ret(T::* func)(Args...) const)
	{
		const char* className = ClassNameProxy::GetLuaClassName<T>();
		typedef Ret(T::*MemberFunctionType)(Args ...) const;
		lua_getglobal(this->mLua, className);
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, name);
			new(lua_newuserdata(this->mLua, sizeof(MemberFunctionType))) MemberFunctionType(func);
			lua_pushcclosure(this->mLua, MemberFunction::MemberFuncProxy<T, Ret, Args...>::Invoke, 1);
			lua_settable(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	template<typename T, typename BC, typename Ret, typename ...Args>
	inline void ClassProxyHelper::PushBaseMemberFunction(const char* name, Ret(BC::* func)(Args...))
	{
		const char* className = ClassNameProxy::GetLuaClassName<T>();
		typedef Ret(BC::*MemberFunctionType)(Args ...);
		lua_getglobal(this->mLua, className);
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, name);
			new(lua_newuserdata(this->mLua, sizeof(MemberFunctionType))) MemberFunctionType(func);
			lua_pushcclosure(this->mLua, MemberFunction::MemberFuncProxy<BC, Ret, Args...>::Invoke, 1);
			lua_settable(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}

	template<typename Ret, typename ... Args>
	void ClassProxyHelper::PushStaticFunction(const char* name, Ret(* func)(Args ...))
	{
		typedef Ret(* StaticFunctionType)(Args ...);
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			lua_pushstring(this->mLua, name);
			new(lua_newuserdata(this->mLua, sizeof(StaticFunctionType))) StaticFunctionType(func);
			lua_pushcclosure(this->mLua, StaticFunction::StaticFuncProxy<Ret, Args ...>::Invoke, 1);
			lua_settable(this->mLua, -3);
		}
		lua_pop(this->mLua, 1);
	}
	template<typename T>
	void ClassProxyHelper::PushObject(T * obj)
	{
		size_t size = sizeof(PtrProxy<T>);
		new(lua_newuserdata(this->mLua, size))PtrProxy<T>(obj);
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			lua_setmetatable(this->mLua, -2);
		}
		lua_setglobal(this->mLua, this->mName.c_str());
	}

	template<typename T>
	void ClassProxyHelper::PushMember(const char* name, const T& value)
	{
		lua_getglobal(this->mLua, this->mName.c_str());
		if (lua_istable(this->mLua, -1))
		{
			Lua::Parameter::Write<T>(this->mLua, value);
			lua_setfield(this->mLua, -2, name);
		}
	}
}