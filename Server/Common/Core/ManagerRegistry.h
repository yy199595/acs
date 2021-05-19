#pragma once
#include<Manager/Manager.h>
namespace SoEasy
{
	class ManagerRegistry
	{
	public:
		template<typename T>
		static bool RegisterManager(const std::string name);
		static Manager * Create(const std::string name);
		static bool GetTypeName(const size_t hash, std::string & name);
	private:
		template<typename T>
		static Manager * CreateManager();
	private:
		static std::unordered_map<size_t, std::string> mTypeInfoMap;
		static std::unordered_map<std::string, std::function<Manager *()>> mCreateActions;
	};
	template<typename T>
	inline bool ManagerRegistry::RegisterManager(const std::string name)
	{
		size_t hash = typeid(T).hash_code();
		auto iter = mCreateActions.find(name);
		if (iter != mCreateActions.end())
		{
			return false;
		}
		mTypeInfoMap.emplace(hash, name);
		mCreateActions.emplace(name, std::bind(&ManagerRegistry::CreateManager<T>));
		return true;
	}
	
	template<typename T>
	inline Manager * ManagerRegistry::CreateManager()
	{
		return new T();
	}

}