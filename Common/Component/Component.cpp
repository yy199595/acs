#include"Component.h"
#include<Object/GameObject.h>
namespace GameKeeper
{
	Component::Component()
		: gameObject(nullptr)
	{
		this->mType = nullptr;
		this->gameObjectID = 0;
	}
	
	Component * Component::GetByHash(size_t hash)
	{
		Type * type = ComponentHelper::GetType(hash);
		if (type == nullptr)
		{
			return nullptr;
		}
		return this->gameObject->GetComponentByName(type->Name);
	}

    Component *Component::GetByName(const std::string &name)
    {
        return this->gameObject->GetComponentByName(name);
    }

    void Component::GetComponents(std::vector<Component *> &components)
    {
        this->gameObject->GetComponents(components);
    }

}