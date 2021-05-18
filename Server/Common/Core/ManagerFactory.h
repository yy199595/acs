#pragma once
#include<Manager/Manager.h>
namespace SoEasy
{
	class ManagerFactory
	{
	public:
		template<typename T>
		bool RegisterManager(const std::string name);
		Manager * Create(const std::string name);
		bool GetTypeName(const size_t hash, std::string & name);
	private:
		template<typename T>
		Manager * CreateManager();
	private:
		std::unordered_map<size_t, std::string> mTypeInfoMap;
		std::unordered_map<std::string, std::function<Manager *()>> mCreateActions;
	};
	template<typename T>
	inline bool ManagerFactory::RegisterManager(const std::string name)
	{
		size_t hash = typeid(T).hash_code();
		auto iter = this->mCreateActions.find(name);
		if (iter != this->mCreateActions.end())
		{
			return false;
		}
		this->mTypeInfoMap.emplace(hash, name);
		this->mCreateActions.emplace(name, std::bind(&ManagerFactory::CreateManager<T>, this));
		return true;
	}
	
	template<typename T>
	inline Manager * ManagerFactory::CreateManager()
	{
		return new T();
	}

}