#pragma once
#include<CommonObject/Object.h>
namespace SoEasy
{
	class GameObject;
	class Component : public Object
	{
	public:
		Component(GameObject *);
		virtual ~Component() {}
	public:
		friend class GameObject;
	public:
		inline long long GetGameObjectID() { return mGameObjectID; }
		inline GameObject * GetGameObject()	{ return this->mGameObject; }
	public:
		template<typename T>
		inline typename T * GetComponent();
		Component * GetComponentByName(const std::string name);
	public:
		bool IsComponent() override { return true; }
	protected:
		virtual void OnFrameStart() { };
		virtual void OnFrameUpdate(float delaTime) { };
		virtual void OnAddComponent(Component * compinent) { };
	private:
		long long mGameObjectID;
		GameObject * mGameObject;
	};
	template<typename T>
	inline typename T * Component::GetComponent()
	{
		const std::string name = TypeReflection<T>::Name;
		Component * pComponent = this->GetComponentByName(name);
		return static_cast<T *>(pComponent);
	}
}