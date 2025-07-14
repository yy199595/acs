#pragma once

#include <string>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>
namespace Lua
{
	namespace ClassNameProxy
	{
		struct ClassRegisterInfo
		{
		public:
			explicit ClassRegisterInfo(std::string str) : mName(std::move(str)) { }
		 public:
			std::string mName;
			std::vector<std::string> mParentNames;
		};

		extern std::unordered_map<size_t, std::unique_ptr<ClassRegisterInfo>> classNameMap;

		template<typename T>
		inline void OnClassRegister(const std::string & name)
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter == classNameMap.end())
			{
				classNameMap.emplace(hash, std::make_unique<ClassRegisterInfo>(name));
			}
		}

		template<typename T>
		inline const char* GetLuaClassName()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter == classNameMap.end())
			{
				return nullptr;
			}
			return iter->second->mName.c_str();
		}

		template<typename T, typename BC>
		inline bool OnPushParent()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = classNameMap.find(hash);
			if (iter != classNameMap.end())
			{
				const char* name = GetLuaClassName<BC>();
				iter->second->mParentNames.emplace_back(name);
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
				return &iter->second->mParentNames;
			}
			return nullptr;
		}
	};// namespace ClassNameProxy
}
