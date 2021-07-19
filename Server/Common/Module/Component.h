#pragma once
#include<Object/GameObject.h>
namespace Sentry
{
	class Component : public Object
	{
	public:
		Component(shared_ptr<GameObject>);
		virtual ~Component() {}
	public:
		friend class GameObject;
		inline long long GetGameObjectID() { return mGameObjectID; }
		inline shared_ptr<GameObject> GetGameObject()	{ return this->mGameObject; }
	public:
		template<typename T>
		inline T * GetComponent();
		Component * GetComponentByName(const std::string name);
	public:
		bool IsComponent() override { return true; }
	protected:
		virtual void OnInit() = 0;
		virtual void OnFrameStart() { };
		virtual void OnFrameUpdate(float delaTime) { };
		virtual void OnAddComponent(Component * compinent) { };
	private:
		long long mGameObjectID;
		shared_ptr<GameObject> mGameObject;
	};
	template<typename T>
	inline T * Component::GetComponent()
	{
		this->mGameObject->GetComponent<T>();
	}
}