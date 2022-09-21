#pragma once
#include"Unit/Object.h"
#include"Component/ComponentFactory.h"
namespace Sentry
{
	class Component;
    typedef std::unordered_map<std::string, Component*>::iterator ComponentIter;
    class Unit : public Object, public std::enable_shared_from_this<Unit>
	{
	 public:
		explicit Unit(long long id);
		Unit(const Unit &) = delete;
		virtual ~Unit() override = default;
	 public:
		template<typename T>
		inline bool AddComponent();
		bool AddComponent(const std::string& name);
		bool AddComponent(const std::string& name, Component* component);

		template<typename T>
		inline T* GetComponent() const;

		template<typename T>
		inline T* GetComponent(const std::string& name) const;

		Component* GetComponentByName(const std::string& name);

		template<typename T>
		inline bool RemoveComponent();

		bool RemoveComponent(const std::string& name);

		template<typename T>
		inline T* GetOrAddComponent();

		template<typename T>
		inline std::shared_ptr<T> Cast();

        ComponentIter ComponentBegin() { return this->mComponentMap.begin();}
        ComponentIter ComponentEnd() { return this->mComponentMap.end();}

	 public:
		void OnDestory() override;
		size_t GetComponents(std::vector<Component*>& components) const;
		size_t GetComponents(std::vector<std::string>& components) const;
	 protected:
		virtual void OnAddComponent(Component * component) {}
		virtual void OnDelComponent(Component * component) {}
	 public:
		inline long long GetUnitId() const
		{
			return this->mUnitId;
		}
	 private:
		long long mUnitId;
		std::vector<std::string> mSortComponents;
		std::unordered_map<std::string, Component*> mComponentMap;
	};
	template<typename T>
	inline T* Unit::GetComponent() const
	{
		Type* type = ComponentFactory::GetType<T>();
		if (type == nullptr)
		{
			return nullptr;
		}
		T* component = this->GetComponent<T>(type->Name);
		if (component != nullptr)
		{
			return component;
		}
		return nullptr;
	}

	template<typename T>
	T* Unit::GetComponent(const std::string& name) const
	{
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component* component = iter->second;
			return dynamic_cast<T*>(component);
		}
		return nullptr;
	}

	template<typename T>
	inline bool Unit::AddComponent()
	{
		if (this->GetComponent<T>() == nullptr)
		{
			Component* component = ComponentFactory::CreateComponent<T>();
			return this->AddComponent(component->GetType()->Name, component);
		}
		return false;
	}

	template<typename T>
	inline bool Unit::RemoveComponent()
	{
		Type* type = ComponentFactory::GetType<T>();
		if (type == nullptr)
		{
			return false;
		}
		return this->RemoveComponent(type->Name);
	}

	template<typename T>
	inline T* Unit::GetOrAddComponent()
	{
		T* component = this->GetComponent<T>();
		if (component == nullptr)
		{
			this->AddComponent<T>();
			return this->GetComponent<T>();
		}
		return component;
	}

	template<typename T>
	inline std::shared_ptr<T> Unit::Cast()
	{
		return std::dynamic_pointer_cast<T>(this->shared_from_this());
	}
}