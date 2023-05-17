#pragma once
#include"Entity/Component/ComponentFactory.h"
namespace Tendo
{
    class Unit
	{
	 public:
		explicit Unit(long long id);
		Unit(const Unit &) = delete;
		Unit(const Unit &&) = delete;
		Unit & operator = (const Unit &) = delete;
	 public:
		template<typename T>
		inline bool AddComponent();
		bool HasComponent(const std::string& name);
		bool AddComponent(const std::string& name);
		bool AddComponent(const std::string& name, std::unique_ptr<Component> component);

		template<typename T>
		inline T* GetComponent() const;

		template<typename T>
		inline T* GetComponent(const std::string& name) const;

		Component* GetComponentByName(const std::string& name) const;

		template<typename T>
		inline bool RemoveComponent();

		bool RemoveComponent(const std::string& name);

		template<typename T>
		inline T* GetOrAddComponent();

		template<typename T>
		inline T * Cast() { return dynamic_cast<T*>(this); }

	 public:
		virtual bool LateAwake() = 0;
		size_t GetComponents(std::vector<Component*>& components) const;
		size_t GetComponents(std::vector<std::string>& components) const;

        template<typename T>
        size_t GetComponents(std::vector<T *> & components) const;
	 protected:
		virtual void OnAddComponent(Component * component) {}
		virtual bool OnDelComponent(Component * component) { return true; }
	 public:
		inline long long GetUnitId() const
		{
			return this->mUnitId;
		}
	 private:
		long long mUnitId;
		std::vector<std::string> mSortComponents;
		std::unordered_map<std::string, std::unique_ptr<Component>> mComponentMap;
	};

    template<typename T>
    size_t Unit::GetComponents(std::vector<T *> &components) const
    {
        for(const std::string & name : this->mSortComponents)
        {
            T * component = this->GetComponent<T>(name);
            if(component != nullptr)
            {
                components.emplace_back(component);
            }
        }
        return components.size();
    }

	template<typename T>
	inline T* Unit::GetComponent() const
	{
		Type* type = ComponentFactory::GetType<T>();
		if (type == nullptr)
		{
			return nullptr;
		}
		return this->GetComponent<T>(type->Name);
	}

	template<typename T>
	T* Unit::GetComponent(const std::string& name) const
	{
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component* component = iter->second.get();
			return dynamic_cast<T*>(component);
		}
		return nullptr;
	}

	template<typename T>
	inline bool Unit::AddComponent()
	{
		if (this->GetComponent<T>() == nullptr)
		{			
			std::unique_ptr<Component> component = ComponentFactory::CreateComponent<T>();
			if (component == nullptr)
			{
				//const char * name = typeid(T).name();
				return false;
			}
			const std::string& name = ComponentFactory::GetName<T>();
			return this->AddComponent(name, std::move(component));
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
			if(!this->AddComponent<T>())
            {
                return nullptr;
            }
			return this->GetComponent<T>();
		}
		return component;
	}
}