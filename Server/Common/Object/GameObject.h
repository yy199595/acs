#pragma once
#include"Object.h"
namespace SoEasy
{
	class Component;
	class GameObject : public Object
	{
	public:
		GameObject(const long long id);
		GameObject(const long long id, const std::string & address);
		virtual ~GameObject() {};
	public:
		template<typename T>
		typename T * AddComponent();

		template<typename T> 
		typename T * GetComponent();

		template<typename T> 
		typename bool RemoveComponent();
		
		template<typename T> 
		typename TypeReflection<T>::Type * GetOrAddComponent();
	
	public:
		void DestoryComponents();
		Component * AddComponentByName(const std::string name);
		Component * GetComponentByName(const std::string name);
		bool RemoveComponentByName(const std::string & name);
		void GetAllComponent(SayNoArray<Component *> & mConponentArray);
	protected:
		void OnDestory() override;
	public:
		inline const std::string & GetBindAddress() { return this->mSessionAddress; }
		inline const long long GetGameObjectID() const { return this->mGameObjectId; }
	private:
		void PollComponent(float t);
	private:
		long long mGameObjectId;
		std::string mSessionAddress;
		SayNoQueue<Component *> mWaitStartComponents;
		SayNoHashMap<std::string, Component *> mComponentMap;
	private:
		typedef SayNoHashMap<std::string, Component *>::iterator ComponentIter;
	};
	typedef shared_ptr<GameObject> SharedGameObject;

	template<typename T>
	inline typename T * GameObject::GetComponent()
	{
		std::string name;
		if (!SoEasy::GetTypeName<T>(name))
		{
			SayNoDebugError("use 'TYPE_REFLECTION' register type:" << typeid(T).name());
			return false;
		}

		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component * component = iter->second;
			return static_cast<T*>(component);
		}
		auto iter1 = this->mComponentMap.begin();
		for (; iter1 != this->mComponentMap.end(); iter1++)
		{
			T * component = dynamic_cast<T*>(iter->second);
			if (component != nullptr)
			{
				return component;
			}
		}
		return nullptr;
	}

	template<typename T>
	inline typename T * GameObject::AddComponent()
	{
		Component * component = this->GetComponent<T>();
		if (component == nullptr)
		{
			component = ObjectFactory::Get()->CreateObject<T>(this);
			if (component != nullptr)
			{
				this->mWaitStartComponents.push(component);
				const std::string & name = component->GetTypeName();
				this->mComponentMap.insert(std::make_pair(name, component));
			}
		}
		return static_cast<T*>(component);
	}

	template<typename T>
	inline typename bool GameObject::RemoveComponent()
	{
		const char * name = TypeReflection<T>::Name;
		ComponentIter iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component * component = iter->second;
			component->SetActive(false);
			return true;
		}
		return false;
	}

	template<typename T>
	inline typename TypeReflection<T>::Type * GameObject::GetOrAddComponent()
	{
		T * component = this->GetComponent<T>();
		return component == nullptr ? this->AddComponent<T>() : component;
	}
}