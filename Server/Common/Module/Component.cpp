#include"Component.h"
#include<Object/GameObject.h>

namespace Sentry
{
	Component::Component(shared_ptr<GameObject> obj)
		: mGameObject(obj)
	{
		this->mGameObjectID = obj->GetGameObjectID();
	}

	Component * Component::GetComponentByName(const std::string name)
	{
		SayNoAssertRetNull_F(mGameObject);
		return mGameObject->GetComponentByName(name);
	}
}