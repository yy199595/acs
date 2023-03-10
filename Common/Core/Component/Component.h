#pragma once

#include"IComponent.h"
#include"Log/CommonLogDef.h"
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
		virtual std::unique_ptr<Component> New() = 0;
	public:
		const size_t Hash;
		const std::string Name;
	};
}

namespace Sentry
{
	class Unit;
	class Component
	{
	 public:
		Component();
		Component(const Component &) = delete;
	public:
		friend class Unit;
		friend class ComponentFactory;
		inline Unit * GetUnit() { return this->mUnit; }
		inline const std::string& GetName() { return this->mName; }
        inline long long GetUnitId() const { return this->mEntityId; }
        template<typename T>
		inline T* Cast() { return dynamic_cast<T*>(this); }

	 public:

		virtual bool Awake() { return true; }; //组件创建的时候调用

		virtual bool LateAwake() { return true;}; // 所有组件加载完成之后调用

		virtual void OnDestroy() { }
	protected:
		template<typename T>
		T* GetComponent();

		template<typename T>
		T* GetComponent(const std::string& name);

		Component* GetByName(const std::string& name);

	 private:
		Component* GetByHash(size_t hash);
	private:
        std::string mName;
		long long mEntityId;
    protected:
        App * mApp;
        Unit * mUnit;
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