#pragma once

#include<CommonProtocol/Common.pb.h>
using namespace google::protobuf;

namespace SoEasy
{
	class Manager;
	class Component;
	class Applocation;
	class ScriptManager;
	class CoroutineManager;
}

namespace SoEasy
{
	
	typedef std::function<Manager *()> CreateMgrAction;
	typedef std::function<Component *()> CreateComAction;

	class Type
	{
	public:
		Type(const std::string name, CreateMgrAction action);
		Type(const std::string name, CreateComAction action);

	public:
		Manager * NewManager(Applocation * app);
		Component * NewComponent(Applocation * app);

	public:
		const std::string & GetTypeName() { return this->nTypeName; }
	private:
		bool mIsManager;
		bool mIsComponent;
		long long mTypeId;
		std::string nTypeName;
		CreateMgrAction mBindCreateMgrAction;
		CreateComAction mBindCreateComAction;
	};

	template<typename T>
	inline Manager * CreatManagerProxy() { return new T(); }

	template<typename T>
	inline Component * CreatComponentProxy() { return new T(); }

	class Factory
	{
	
	public:
		template<typename T>
		Manager * CreateManager(Applocation * app);
		Manager * CreateManager(Applocation * app, std::string name);

		template<typename T>
		Component * CreateComponent(Applocation * app);
		Component * CreateComponent(Applocation * app, std::string name);
	public:
		template<typename T>
		Type * GetType();
		Type * GetType(const std::string name);
	public:
		static Factory * GetPtr();
	public:
		bool InitRegisterObject();
	public:
		template<typename T>
		bool RegisterManager(const std::string name);
	public:
		Message * CreateMessage(const std::string & name);
	public:
		std::unordered_map<std::string, Message *> mMessageMap;
		std::unordered_map<size_t, Type *> mRegisterClassIdMap;
		std::unordered_map<std::string, Type *> mRegisterClassNameMap;

	private:
		static Factory * mObjectFactory;
	};

	template<typename T>
	inline Manager * Factory::CreateManager(Applocation * app)
	{
		Type * type = this->GetType<T>();
		return type != nullptr ? type->NewManager(app) : nullptr;
	}

	template<typename T>
	inline Component * Factory::CreateComponent(Applocation * app)
	{
		Type * type = this->GetType<T>();
		return type != nullptr ? type->NewComponent(app) : nullptr;
	}

	template<typename T>
	inline Type * Factory::GetType()
	{
		size_t key = typeid(T).hash_code();
		auto iter = this->mRegisterClassIdMap.find(key);
		return iter != this->mRegisterClassIdMap.end() ? iter->second : nullptr;
	}

	template<typename T>
	inline bool Factory::RegisterManager(const std::string name)
	{
		/*size_t key = typeid(T).hash_code();
		auto iter1 = this->mRegisterClassIdMap.find(key);
		auto iter2 = this->mRegisterClassNameMap.find(name);
		
		if (iter1 != this->mRegisterClassIdMap.end())
		{
			return false;
		}
		if (iter2 != this->mRegisterClassNameMap.end())
		{
			return false;
		}*/

		Type * type = new Type(name, CreatManagerProxy<T>);
		this->mRegisterClassIdMap.insert(std::make_pair(0, type));
		this->mRegisterClassNameMap.insert(std::make_pair(name, type));
		return true;
	}
}