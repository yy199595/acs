#pragma once
#include <unordered_map>
#include <Component/Component.h>
namespace Sentry
{
	class Type
	{
	public:
		Type(size_t hash, std::string name) :
			Hash(hash), Name(name) { }
	public:
		virtual Component * Create() = 0;
	public:
		const size_t Hash;
		const std::string Name;
	};

	template<typename T>
	class TypeProxy : public Type
	{
	public:
		TypeProxy(std::string name) : Type(typeid(T).hash_code(), name) { }
	public:
		Component * Create() final { return new T(); }
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
			mTypeInfoMap.insert(std::make_pair(type->Name, type));	
			mTypeInfoMap1.insert(std::make_pair(typeid(T).hash_code(), type));
			return true;
		}

		template<typename T>
		static Type * Get()
		{
			size_t hash = typeid(T).hash_code();
			auto iter = mTypeInfoMap1.find(hash);
			return iter != mTypeInfoMap1.end() ? iter->second : nullptr;
		}
		static Type * Get(const std::string name);
	private:
		static std::unordered_map<size_t, Type *> mTypeInfoMap1;
		static std::unordered_map<std::string, Type *> mTypeInfoMap;

	};
}// namespace Sentry