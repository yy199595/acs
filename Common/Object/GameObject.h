#pragma once

#include<Component/ComponentHelper.h>
namespace Sentry
{
    class Component;
    class GameObject : public Object
    {
    public:
        GameObject(const long long id);

        GameObject(const long long id, const std::string &address);

        virtual ~GameObject() {};
    public:
        template<typename T>
        inline bool AddComponent();
		bool AddComponent(const std::string & name);
		bool AddComponent(const std::string name, Component * component);
		
        template<typename T>
        inline T *GetComponent() const;

		template<typename T>
		inline T *GetComponent(const std::string & name) const;

        template<typename T>
        inline bool RemoveComponent();

        bool RemoveComponent(const std::string &name);

        template<typename T>
        inline T *GetOrAddComponent();

    public:
		void OnDestory() override;
        void GetComponents(std::vector<Component *> & components) const;
    public:
		inline const long long GetId() const { return this->mGameObjectId; }
        inline const std::string &GetAddress() { return this->mSessionAddress; }

    private:
        long long mGameObjectId;
        std::string mSessionAddress;    
        std::unordered_map<std::string, Component *> mComponentMap;
    };
    template<typename T>
    inline T *GameObject::GetComponent() const
    {
		Type * type = ComponentHelper::GetType<T>();
		if (type == nullptr)
		{
            return nullptr;
		}
        return this->GetComponent<T>(type->Name);
    }

	template<typename T>
	T * GameObject::GetComponent(const std::string & name) const
    {
        auto iter = this->mComponentMap.find(name);
        if (iter != this->mComponentMap.end())
        {
            Component *component = iter->second;
            return static_cast<T *>(component);
        }
        return nullptr;
    }

	template<typename T>
	inline bool GameObject::AddComponent()
	{
		if (this->GetComponent<T>() == nullptr)
		{			
			Component * component = ComponentHelper::CreateComponent<T>();		
			return this->AddComponent(component->GetTypeName(), component);
		}
		return false;
	}

	template<typename T>
    inline bool GameObject::RemoveComponent()
    {
		Type * type = ComponentHelper::GetType<T>();
		if (type == nullptr)
		{
			return false;
		}
		return this->RemoveComponent(type->Name);
    }

    template<typename T>
    inline T *GameObject::GetOrAddComponent()
    {
        T *component = this->GetComponent<T>();
        return component == nullptr ? this->AddComponent<T>() : component;
    }
}