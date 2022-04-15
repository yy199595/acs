#pragma once

#include<Component/ComponentFactory.h>
namespace Sentry
{
	class Component;
	class Entity : public Object, public std::enable_shared_from_this<Entity>
	{
	 public:
		Entity(long long id);

		Entity(long long id, long long socketId);

		~Entity() override = default;
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

	 public:
		void OnDestory() override;

		void GetComponents(std::vector<std::string>& components) const;
	 protected:
		virtual void OnAddComponent(Component * component) {}
		virtual void OnDelComponent(Component * component) {}
	 public:
		inline long long GetId() const
		{
			return this->mGameObjectId;
		}
		inline long long GetSocketId() const
		{
			return this->mSocketId;
		}
	 private:
		long long mSocketId;
		long long mGameObjectId;
		std::vector<std::string> mSortComponents;
		std::unordered_map<std::string, Component*> mComponentMap;
	};
	template<typename T>
	inline T* Entity::GetComponent() const
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
	T* Entity::GetComponent(const std::string& name) const
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
	inline bool Entity::AddComponent()
	{
		if (this->GetComponent<T>() == nullptr)
		{
			Component* component = ComponentFactory::CreateComponent<T>();
			return this->AddComponent(component->GetType()->Name, component);
		}
		return false;
	}

	template<typename T>
	inline bool Entity::RemoveComponent()
	{
		Type* type = ComponentFactory::GetType<T>();
		if (type == nullptr)
		{
			return false;
		}
		return this->RemoveComponent(type->Name);
	}

	template<typename T>
	inline T* Entity::GetOrAddComponent()
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
	inline std::shared_ptr<T> Entity::Cast()
	{
		return std::dynamic_pointer_cast<T>(this->shared_from_this());
	}
}