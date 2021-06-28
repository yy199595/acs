#pragma once
#include<Object/Object.h>
#include<Protocol/com.pb.h>
#include<Protocol/s2s.pb.h>
using namespace google::protobuf;
namespace SoEasy
{
	template<typename T>
	inline Object * CreateNullArgcObject() { return new T(); }

	typedef std::function<Object *()> CreateAction;
	class ObjectFactory
	{
	private:
		ObjectFactory(size_t size) :mQueueCount(size) { }
	public:
		static ObjectFactory * Get();
	public:
		template<typename ... Args>
		Object * CreateObjectByName(std::string name, Args &&... args);
		template<typename T, typename ... Args>
		T * CreateObject(Args &&... args);
	public:
		template<typename T>
		bool RegisterObject();
	private:
		size_t mQueueCount;
		std::unordered_map<std::string, CreateAction> mRegisterClassMap;
	};
	template<typename ...Args>
	inline Object * ObjectFactory::CreateObjectByName(std::string name, Args &&...args)
	{
		
		return nullptr;
	}

	template<typename T, typename ...Args>
	inline T * ObjectFactory::CreateObject(Args && ...args)
	{
		std::string className = TypeReflection<T>::Name;
		T * pObject = new T(std::forward<Args>(args)...);
		if (pObject != nullptr)
		{
			pObject->Init(className);
		}
		return pObject;
	}
	template<typename T>
	inline bool ObjectFactory::RegisterObject()
	{
		std::string name = TypeReflection<T>::Name;
		auto iter = this->mRegisterClassMap.find(name);
		if (iter != this->mRegisterClassMap.end())
		{
			return false;
		}
		CreateAction action = CreateNullArgcObject<T>;
		this->mRegisterClassMap.insert(std::make_pair(name, action));
		return true;
	}
}