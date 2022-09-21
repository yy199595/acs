#pragma once

#include"IComponent.h"

#include"XCode/XCode.h"
#include"Unit/Object.h"
#include"Message/db.pb.h"
#include"Message/c2s.pb.h"
#include"Message/com.pb.h"
#include"Message/s2s.pb.h"
#include"Define/CommonLogDef.h"
#include"Method/EventMethod.h"
using namespace google::protobuf;
namespace Sentry
{
	class App;
	class Component;
	class Type
	{
	public:
		Type(size_t hash, std::string name) :
			Hash(hash), Name(std::move(name)) { }
	public:
		virtual Component * New() = 0;
	public:
		const size_t Hash;
		const std::string Name;
	};
}

namespace Sentry
{
	class Unit;
	class ServerConfig;
	class Component : public Object
	{
	 public:
		Component();
		~Component() override = default;
		Component(const Component &) = delete;

	public:
		friend class Unit;
		friend class ComponentFactory;
		inline long long GetEntityId() const
		{
			return this->mEntityId;
		}

		inline std::shared_ptr<Unit> GetUnit()
		{
			return this->mEntity;
		}
		inline Type* GetType()
		{
			return this->mType;
		}

		inline const std::string& GetName()
		{
			return this->mName;
		}
		template<typename T>
		inline T* Cast()
		{
			return dynamic_cast<T*>(this);
		}

		template<typename T>
		inline bool Is()
		{
			return this->mType->Hash == typeid(T).hash_code();
		}

	 public:

		virtual void Awake() { }; //组件创建的时候调用

		virtual bool LateAwake() { return true;}; // 所有组件加载完成之后调用

	protected:
		template<typename T>
		T* GetComponent();

		template<typename T>
		T* GetComponent(const std::string& name);

		Component* GetByName(const std::string& name);

		std::shared_ptr<App> GetApp() { return mApp;}

		const ServerConfig & GetConfig();

	 private:
		Component* GetByHash(size_t hash);
	private:
		Type* mType;
		std::string mName;
		long long mEntityId;
		std::shared_ptr<App> mApp;
		std::shared_ptr<Unit> mEntity;
	};
	template<typename T>
	inline T* Component::GetComponent()
	{
		const size_t hash = typeid(T).hash_code();
		return static_cast<T*>(this->GetByHash(hash));
	}

	template<typename T>
	inline T* Component::GetComponent(const std::string& name)
	{
		Component* component = this->GetByName(name);
		if (component == nullptr)
		{
			return nullptr;
		}
		return dynamic_cast<T*>(component);
	}

	inline std::string GetFunctionName(const std::string func)
	{
		size_t pos = func.find("::");
		return func.substr(pos + 2);
	}
}