#pragma once
#include<type_traits>
namespace SoEasy
{
	template<typename T>
	struct TypeReflection
	{
	public:
		static constexpr bool Value = false;
		static constexpr char * Name = nullptr;
	};
//#define TYPE_REFLECTION(type, base) template<> struct TypeReflection<type, base> { static constexpr char * Name = (char*)#type; static constexpr bool Value = true; typedef typename std::enable_if<(std::is_base_of<base, type>::value), type>::type Type;}

	//template<typename T, typename Base>
	
#define TYPE_REFLECTION(T,name)					\
	template<> struct TypeReflection<T>	\
	{												\
	public:											\
		static constexpr bool Value = true;		\
		static constexpr char * Name = (char*)name;		\
		static constexpr size_t Size = sizeof(T);	\
	}
#define ChildType(type, base) typename std::enable_if<std::is_base_of<base,type>::value && TypeReflection<type>::Value, type>::type
}

namespace SoEasy
{
	template<typename T>
	inline bool GetTypeName(std::string & name) 
	{ 
		if (TypeReflection<T>::Value)
		{
			name.assign(TypeReflection<T>::Name);
			return true;
		}
		return false;
	}
}

namespace SoEasy
{
	class Object;
	class Manager;
	class NetWorkManager;
	class ScriptManager;
	class Component;
	class RemoteScheduler;
	class TcpClientSession;
	class TcpSessionListener;
	class GameObject;
	class ActionManager;
	class ActionRegisterManager;

}