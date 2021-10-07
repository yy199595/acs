#include"Component.h"
#include<Object/GameObject.h>
namespace Sentry
{
	Component::Component()
		: gameObject(nullptr)
	{
		this->mType = nullptr;
		this->gameObjectID = 0;
	}
	
	Component * Component::GetComponentByHash(const size_t hash)
	{
		Type * type = ComponentHelper::GetType(hash);
		if (type == nullptr)
		{
			return nullptr;
		}
		return this->gameObject->GetComponentByName(type->Name);
	}
}