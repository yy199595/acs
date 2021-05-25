#pragma once
#include<Manager/Manager.h>
#include<Service/ServiceBase.h>
namespace SoEasy
{
	template<typename Base>
	class ObjectRegistry
	{
	public:
		template<typename T>
		static bool Register(const std::string name);
		static Base * Create(const std::string name);
		static bool GetTypeName(const size_t hash, std::string & name);
	private:
		template<typename T>
		static Base * Create();
	private:
		static std::unordered_map<size_t, std::string> mTypeInfoMap;
		static std::unordered_map<std::string, std::function<Base *()>> mCreateActions;
	};

	template<typename Base>
	std::unordered_map<size_t, std::string> ObjectRegistry<Base>::mTypeInfoMap;
	template<typename Base>
	std::unordered_map<std::string, std::function<Base *()>> ObjectRegistry<Base>::mCreateActions;

	template<typename Base>
	template<typename T>
	inline bool ObjectRegistry<Base>::Register(const std::string name)
	{
		size_t hash = typeid(T).hash_code();
		auto iter = mCreateActions.find(name);
		if (iter != mCreateActions.end())
		{
			return false;
		}
		mTypeInfoMap.emplace(hash, name);
		mCreateActions.emplace(name, std::bind(&ObjectRegistry::Create<T>));
		return true;
	}
	template<typename Base>
	template<typename T>
	inline Base * ObjectRegistry<Base>::Create()
	{
		return new T();
	}

	template<typename Base>
	inline Base * ObjectRegistry<Base>::Create(const std::string name)
	{
		auto iter = mCreateActions.find(name);
		return iter != mCreateActions.end() ? iter->second() : nullptr;
	}

	template<typename Base>
	inline bool ObjectRegistry<Base>::GetTypeName(const size_t hash, std::string & name)
	{
		auto iter = mTypeInfoMap.find(hash);
		if (iter != mTypeInfoMap.end())
		{
			name = iter->second;
			return true;
		}
		return false;
	}

}