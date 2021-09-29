#pragma once
#include <unordered_map>
#include <Component/Component.h>
namespace Sentry
{
	template<typename T>
	class TypeProxy : public Type
	{
	public:
		TypeProxy(std::string name) : Type(typeid(T).hash_code(), name) { }
	public:
		Component * New() final { return new T(); }
	};

	class ComponentHelper
	{
	public:
		template<typename T>
		static bool Add(const std::string name)
		{
			auto iter = mTypeInfoMap.find(name);
			if (iter != mTypeInfoMap.end())
			{
				return false;
			}
			TypeProxy<T> * type = new TypeProxy<T>(name);
			if (type != nullptr)
			{
				std::queue<Component *> components;
				mComponentPool.emplace(type->Hash, components);
				mTypeInfoMap.insert(std::make_pair(type->Name, type));
				mTypeInfoMap1.insert(std::make_pair(typeid(T).hash_code(), type));
			}
			return true;
		}

		template<typename T>
		static Type * GetType()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = mTypeInfoMap1.find(hash);
			return iter != mTypeInfoMap1.end() ? iter->second : nullptr;
		}

		static bool DestoryComponent(Component * component);
		template<typename T>
		static Component * CreateComponent(bool fromPool = true);
		static Component * CreateComponent(const std::string & name, bool fromPool = true);

		static Type * GetType(const std::string name);
	private:
		static std::unordered_map<size_t, Type *> mTypeInfoMap1;
		static std::unordered_map<std::string, Type *> mTypeInfoMap;
		static std::unordered_map<size_t, std::queue<Component *>> mComponentPool;

	};

	template<typename T>
	Component * Sentry::ComponentHelper::CreateComponent(bool fromPool /*= true*/)
	{
		size_t key = typeid(T).hash_code();
		auto iter = mTypeInfoMap1.find(key);
		if (iter == mTypeInfoMap1.end())
		{			
			return nullptr;
		}
		Type * type = iter->second;
		return CreateComponent(type->Name);
	}
#define __register_component__(type) ComponentHelper::Add<type>(#type)
}// namespace Sentry