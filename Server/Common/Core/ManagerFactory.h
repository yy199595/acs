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
	private:
		template<typename T>
		Manager * CreateManager();
	private:
		std::unordered_map<std::string, std::function<Manager *()>> mCreateActions;
	};
	template<typename T>
	inline bool ManagerFactory::RegisterManager(const std::string name)
	{
		auto iter = this->mCreateActions.find(name);
		if (iter != this->mCreateActions.end())
		{
			return false;
		}
		this->mCreateActions.emplace(name, std::bind(&ManagerFactory::CreateManager<T>, this));
		return true;
	}
	template<typename T>
	inline Manager * ManagerFactory::CreateManager()
	{
		return new T();
	}

}