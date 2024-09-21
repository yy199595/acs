#include"Component.h"
#include"Entity/Actor/App.h"
namespace acs
{
	Component::Component()
		: mUnit(nullptr)
	{
		this->mEntityId = 0;
		this->mApp = App::Inst();
	}

	Component* Component::GetByHash(size_t hash)
	{
		Type* type = ComponentFactory::GetType(hash);
		if (type == nullptr)
		{
			return nullptr;
		}
		return this->mUnit->GetComponentByName(type->Name);
	}

	Component* Component::GetByName(const std::string& name)
	{
		return this->mUnit->GetComponentByName(name);
	}
}