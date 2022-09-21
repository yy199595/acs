#pragma once

#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace Lua
{
	namespace ClassNameProxy
	{
		struct ClassRegisterInfo
		{
		 public:
			std::string mName;
			std::vector<std::string> mParentNames;
		};

		extern std::unordered_map<size_t, ClassRegisterInfo*> classNameMap;
		typedef std::unordered_map<size_t, ClassRegisterInfo*>::iterator ClassNameMapIter;

		template<typename T>
		inline void OnClassRegister(const std::string name)
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter == classNameMap.end())
			{
				auto info = new ClassRegisterInfo();
				info->mName = name;
				classNameMap.emplace(hash, info);
			}
		}

		template<typename T>
		inline const char* GetLuaClassName()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter != classNameMap.end())
			{
				ClassRegisterInfo* info = iter->second;
				return info->mName.c_str();
			}
			return nullptr;
		}

		template<typename T, typename BC>
		inline bool OnPushParent()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter != classNameMap.end())
			{
				ClassRegisterInfo* info = iter->second;
				const char* name = GetLuaClassName<BC>();
				info->mParentNames.emplace_back(name);
				return true;
			}
			return false;
		}

		template<typename T>
		inline bool HasRegisterClass()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			return iter != classNameMap.end();
		}

		template<typename T>
		inline std::vector<std::string>* GetParents()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter != classNameMap.end())
			{
				ClassRegisterInfo* info = iter->second;
				return &info->mParentNames;
			}
			return nullptr;
		}
	};// namespace ClassNameProxy
}
