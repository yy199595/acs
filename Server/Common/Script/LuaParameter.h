#pragma once

#include"LuaInclude.h"
#include"UserDataParameter.h"
#include"FunctionParameter.h"
#include"ContainerParameter.h"
#include"CommonParameter.h"
#include"ProtocParameter.h"

namespace LuaParameter
{
	template<typename T>
	inline typename std::enable_if<std::is_enum<T>::value, T>::type Read(lua_State * lua, int index)
	{
		return (T)lua_tointeger(lua, index);
	}
	template<typename T>
	inline typename std::enable_if<std::is_enum<T>::value, void>::type Write(lua_State * lua, T data)
	{
		lua_pushinteger(lua, (int)data);
	}
}

//namespace LuaParameter
//{
//	template<typename T>
//	inline typename std::enable_if<ConstParameter::IsConstParameter<T>::value, T>::type Read(lua_State * lua, int index)
//	{
//		typedef ConstParameter::ConstProxy<T>::Type Type;
//		return LuaParameter::Read<Type>(lua, index);
//	}
//	template<typename T>
//	inline typename std::enable_if<ConstParameter::IsConstParameter<T>::value, void>::type Write(lua_State * lua, T data)
//	{
//		typedef ConstParameter::ConstProxy<T>::Type Type;
//		LuaParameter::Write<Type>(lua, data);
//	}
//}

namespace LuaParameter
{
	//普通数据类型
	template<typename T>
	inline typename std::enable_if<CommonParameter::IsCommonParameter<T>::value, void>::type Write(lua_State * lua, T data)
	{
		typedef typename ConstParameter::ConstProxy<T>::Type Type;
		CommonParameter::Write<Type>(lua, data);
	}

	template<typename T>
	inline typename std::enable_if<CommonParameter::IsCommonParameter<T>::value, T>::type Read(lua_State * lua, int index)
	{
		return CommonParameter::Read<T>(lua, index);
	}
}

namespace LuaParameter
{
	//容器数据类型
	template<typename T>
	inline typename std::enable_if<ContainerParameter::IsContainerParameter<T>::value, void>::type Write(lua_State * lua, T data)
	{
		ContainerParameter::Write<T>(lua, data);
	}

	template<typename T>
	inline typename std::enable_if<ContainerParameter::IsContainerParameter<T>::value, T>::type Read(lua_State * lua, int index)
	{
		return ContainerParameter::Read<T>(lua, index);
	}
}

namespace LuaParameter
{
	//函数数据类型
	template<typename T>
	inline typename std::enable_if<FunctionParameter::IsFunctionParameter<T>::value, T>::type Read(lua_State * lua, int index)
	{
		return FunctionParameter::Read<T>(lua, index);
	}

	template<typename T>
	inline typename std::enable_if<FunctionParameter::IsFunctionParameter<T>::value, void>::type Write(lua_State * lua, T data)
	{

	}
}

namespace LuaParameter
{
	template<typename T>
	inline typename std::enable_if<ProtocParameter::IsProtocParameter<T>::value, T>::type Read(lua_State * lua, int index)
	{
		return ProtocParameter::Read<T>(lua, index);
	}
	template<typename T>
	inline typename std::enable_if<ProtocParameter::IsProtocParameter<T>::value, void>::type Write(lua_State * lua, T data)
	{
		ProtocParameter::Write<T>(lua, data);
	}
}

namespace LuaParameter
{
	// user data类型
	template<typename T>
	inline typename std::enable_if<UserDataParameter::IsUserDataParameter<T>::value, T>::type Read(lua_State * lua, int index)
	{
		return UserDataParameter::Read<T>(lua, index);
	}

	template<typename T>
	inline typename std::enable_if<UserDataParameter::IsUserDataParameter<T>::value, void>::type Write(lua_State * lua, T data)
	{
		UserDataParameter::Write<T>(lua, data);
	}
}

namespace LuaParameter
{
	inline void Encode(lua_State * lua) {}

	template<typename T, typename ... Args>
	inline void Encode(lua_State * lua, const T & t, Args ... args)
	{
		LuaParameter::Write<T>(lua, t);
		Encode(lua, std::forward<Args>(args) ...);
	}

	template<typename ...Args>
	inline void WriteArgs(lua_State * lua, Args ... args)
	{
		Encode(lua, std::forward<Args>(args)...);
	}
}